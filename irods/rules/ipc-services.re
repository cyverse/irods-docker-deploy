# VERSION 2
#
# ipc-services.re
# This is a library of rules to support service specific policies.

_ipc_HOME = '/' ++ ipc_ZONE ++ '/home'

#
# These are the constants used by iRODS to identity the type of an entity.
#

# Identifies a collection
ipc_COLLECTION: string
ipc_COLLECTION = '-C'

# Identifies a data object
ipc_DATA_OBJECT: string
ipc_DATA_OBJECT = '-d'

# Identifies a resource
ipc_RESOURCE: string
ipc_RESOURCE = '-R'

# Identifies a user
ipc_USER: string
ipc_USER = '-u'

# tests whether a given entity type identifier indicates a collection
#
# Parameters:
#   *Type - the entity type identifier
#
# NB: Sometimes iRODS passes `-c` to indicate a collection
#
ipc_isCollection: string -> boolean
ipc_isCollection(*Type) = *Type == ipc_COLLECTION || *Type == '-c'

# tests whether a given entity type identifier indicates a data object
#
# Parameters:
#   *Type - the entity type identifier
#
ipc_isDataObject: string -> boolean
ipc_isDataObject(*Type) = *Type == ipc_DATA_OBJECT

# tests whether a given entity type identifier indicates a collection or a data
# object
#
# Parameters:
#   *Type - the entity type identifier
#
ipc_isFileSystemType: string -> boolean
ipc_isFileSystemType(*Type) = ipc_isCollection(*Type) || ipc_isDataObject(*Type)

# tests whether a given entity type identifier indicates a resource
#
# Parameters:
#   *Type - the entity type identifier
#
# NB: Sometimes iRODS passes `-r` to indicated a resource
#
ipc_isResource: string -> boolean
ipc_isResource(*Type) = *Type == ipc_RESOURCE || *Type == '-r'

# tests whether a given entity type identifier indicates a user
#
# Parameters:
#   *Type - the entity type identifier
#
ipc_isUser: string -> boolean
ipc_isUser(*Type) = *Type == ipc_USER

# Looks up the type of an entity
#
# PARAMETERS:
#  Entity  the name of a resource or user or the path of a collection or data
#          object
#
# RETURNS:
#  It returns the type or '' if the type of Entity can't be determined
#
ipc_getEntityType: string -> string
ipc_getEntityType(*Entity) =
  let *type = '' in
  if errormsg(msiGetObjType(*Entity, *type), *err) < 0
  then let *_ = writeLine('serverLog', 'ipc_getEntityType(*Entity) -> *err') in ''
  else
    if ipc_isCollection(*type) then ipc_COLLECTION
    else if ipc_isDataObject(*type) then ipc_DATA_OBJECT
    else if ipc_isResource(*type) then ipc_RESOURCE
    else if ipc_isUser(*type) then ipc_USER
    else *type

# The base collection for staging
ipc_STAGING_BASE: path
ipc_STAGING_BASE = let *zone = ipc_ZONE in /*zone/jobs

# This function checks to see if a collection or data object is in the staging
# collection.
#
# PARAMETERS:
#  Path  the absolute path to the entity
#
# RETURNS:
#  It returns true if then entity is inside staging, otherwise false
ipc_inStaging: path -> boolean
ipc_inStaging(*Path) = str(*Path) like str(ipc_STAGING_BASE) ++ '/*'


# This function checks to see if a collection or data object is inside a user
# collection managed by a service.
#
# PARAMETERS:
#  SvcUser  the iRODS user name used by the service
#  SvcColl  the name of the user collection managed by the service
#  Path     the path to the collection or data object of interest
#
# RETURNS:
#  It returns true if the collection or data object is inside the user
#  collection, otherwise it returns false.
#
ipc_isForService: string * string * path -> boolean
ipc_isForService(*SvcUser, *SvcColl, *Path) =
  let *strPath = str(*Path) in
  *strPath like regex _ipc_HOME ++ '/[^/]+/*SvcColl($|/.*)'
  && !(*strPath like _ipc_HOME ++ '/*SvcUser/*')
  && !(*strPath like _ipc_HOME ++ '/shared/*')
  && !(*strPath like _ipc_HOME ++ '/public/*')


# This rule gives access to a service for a collection and everything in it.
#
# PARAMETERS:
#  SvcUser     the iRODS user name used by the service
#  Permission  the permission to grant. It should be 'null', 'read', 'write', or
#              'own'.
#  CollPath    the path to the collection of begin given write access to
#
ipc_giveAccessColl(*SvcUser, *Permission, *CollPath) {
  writeLine('serverLog',
            'permitting *SvcUser *Permission access to *CollPath and everything in it');

  msiSetACL('recursive', *Permission, *SvcUser, *CollPath);
}


# This rule gives access to a service for a data object.
#
# PARAMETERS:
#  SvcUser     the iRODS user name used by the service
#  Permission  the permission to grant. It should be 'null', 'read', 'write', or
#              'own'.
#  ObjPath     the path to the data object of begin given write access to
#
ipc_giveAccessObj(*SvcUser, *Permission, *ObjPath) {
  writeLine('serverLog', 'permitting *SvcUser write access to *ObjPath');
  msiSetACL('default', *Permission, *SvcUser, *ObjPath);
}


# This rule ensures that a service user gets access to a presumably newly
# created collection if it were created inside a user collection managed by the
# service.
#
# PARAMETERS:
#  SvcUser     the iRODS user name used by the service
#  SvcColl     the name of the user collection managed by the service
#  Permission  the permission to grant. It should be 'null', 'read', 'write', or
#              'own'.
#  CollPath    the path to the collection of interest
#
ipc_ensureAccessOnCreateColl(*SvcUser, *SvcColl, *Permission, *CollPath) {
  if (ipc_isForService(*SvcUser, *SvcColl, /*CollPath)) {
    ipc_giveAccessColl(*SvcUser, *Permission, *CollPath);
  }
}


# This rule ensures that a service user gets access to a presumably newly
# created data object if it were created inside a user collection managed by the
# service.
#
# PARAMETERS:
#  SvcUser     the iRODS user name used by the service
#  SvcColl     the name of the user collection managed by the service
#  Permission  the permission to grant. It should be 'null', 'read', 'write', or
#              'own'.
#  ObjPath     the path to the data object of interest
#
ipc_ensureAccessOnCreateObj(*SvcUser, *SvcColl, *Permission, *ObjPath) {
  if (ipc_isForService(*SvcUser, *SvcColl, /*ObjPath)) {
    ipc_giveAccessObj(*SvcUser, *Permission, *ObjPath);
  }
}


# This rule ensures that a service user gets access to a collection or data
# object if it has been moved into a user collection managed by the service.
#
# PARAMETERS:
#  SvcUser     the iRODS user name used by the service
#  SvcColl     the name of the user collection managed by the service
#  Permission  the permission to grant. It should be 'null', 'read', 'write', or
#              'own'.
#  OldPath     the original iRODS path to the entity
#  NewPath     the new iRODS path to the entity
#
ipc_ensureAccessOnMv(*SvcUser, *SvcColl, *Permission, *OldPath, *NewPath) {
  if (
    !ipc_isForService(*SvcUser, *SvcColl, /*OldPath)
    && ipc_isForService(*SvcUser, *SvcColl, /*NewPath)
  ) {
    *type = ipc_getEntityType(*NewPath);

    if (ipc_isCollection(*type)) {
      ipc_giveAccessColl(*SvcUser, *Permission, *NewPath);
    } else if (ipc_isDataObject(*type)) {
      ipc_giveAccessObj(*SvcUser, *Permission, *NewPath);
    }
  }
}
