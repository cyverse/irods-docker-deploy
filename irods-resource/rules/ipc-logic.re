# The general Data Store rule independent of environment specific rule 
# customizations.
#
# © 2021 The Arizona Board of Regents on behalf of The University of Arizona. 
# For license information, see https://cyverse.org/license.

@include 'ipc-json'
@include 'ipc-uuid'

COLLECTION_TYPE = 'collection'
DATA_OBJECT_TYPE = 'data-object'
RESOURCE_TYPE = 'resource'
USER_TYPE = 'user'


getTimestamp() {
  msiGetSystemTime(*timestamp, 'human')
  *timestamp
}


contains(*item, *list) {
  *result = false;
  foreach (*currItem in *list) {
    if (*item == *currItem) {
      *result = true;
    }
  }
  *result;
}


# Assign a UUID to a given collection or data object.
assignUUID(*ItemType, *ItemName) {
  *uuid = ipc_uuidGenerate;
  writeLine('serverLog', 'UUID *uuid created');
# XXX - This is a workaround for https://github.com/irods/irods/issues/3437. It is still present in
#       4.2.2.
#  msiModAVUMetadata(*ItemType, *ItemName, 'set', 'ipc_UUID', *uuid, '');
  *status = errormsg(msiModAVUMetadata(*ItemType, *ItemName, 'set', 'ipc_UUID', *uuid, ''), *msg);

  if (*status == -818000) {
    # assume it was uploaded by a ticket
    *typeArg = execCmdArg(*ItemType);
    *nameArg = execCmdArg(*ItemName);
    *valArg = execCmdArg(*uuid);
    *argStr = "*typeArg *nameArg *valArg";
    *status = errormsg(msiExecCmd('setuuid.sh', *argStr, "null", "null", "null", *out), *msg);
    if (*status != 0) {
      writeLine('serverLog', "Failed to assign UUID: *msg");
      fail;
    }
  } else if (*status != 0) {
    writeLine('serverLog', "Failed to assign UUID: *msg");
    fail;
  }
# XXX - ^^^

  *uuid;
}


# Looks up the UUID of a collection from its path.
retrieveCollectionUUID(*Coll) {
  *uuid = '';
  *res = SELECT META_COLL_ATTR_VALUE WHERE COLL_NAME == *Coll AND META_COLL_ATTR_NAME == 'ipc_UUID';
  foreach (*record in *res) {
    *uuid = *record.META_COLL_ATTR_VALUE;
  }
  *uuid;
}


# Looks up the UUID of a data object from its path.
retrieveDataUUID(*Data) {
  *uuid = '';
  msiSplitPath(*Data, *parentColl, *dataName);
  *res = SELECT META_DATA_ATTR_VALUE
           WHERE COLL_NAME == *parentColl
             AND DATA_NAME == *dataName
             AND META_DATA_ATTR_NAME == 'ipc_UUID';
  foreach (*record in *res) {
    *uuid = *record.META_DATA_ATTR_VALUE;
  }
  *uuid;
}


# Looks up the UUID for a given type of entity (collection or data object)
retrieveUUID(*EntityType, *EntityPath) {
  if (*EntityType == '-C') {
    retrieveCollectionUUID(*EntityPath);
  } else if (*EntityType == '-d') {
    retrieveDataUUID(*EntityPath);
  } else {
    ''
  }
}


# sends a message to a given AMQP Streaming
#
# Parameters:
#  *Topic  (string) the topic of the message
#  *Msg    (string) the message to send
#
# Remote Execution:
#  It executes the irods-rule-async-exec-cmd command on the rule engine host
#
sendMsg(*Topic, *Msg) {
  *topicArg = execCmdArg(*Topic);
  *msgArg = execCmdArg(*Msg);
  *argStr = 'send_msg *topicArg *msgArg';

  *status = errormsg(
    msiExecCmd('irods-rule-async-exec-cmd', *argStr, 'null', 'null', 'null', *out), *errMsg );

  if (*status < 0) {
    writeLine("serverLog", "Failed to send message: *errMsg");
  }

  0;
}


mkUserObject(*Field, *Name, *Zone) =
  if (*Zone == '') then
    ipcJson_object(
      *Field, list(ipcJson_string('name', *Name), ipcJson_string('zone', $rodsZoneClient)) )
  else
    ipcJson_object(*Field, list(ipcJson_string('name', *Name), ipcJson_string('zone', *Zone)))


_ipc_mkAuthorField(*Name, *Zone) = mkUserObject('author', *Name, *Zone)


mkEntityField(*UUID) = ipcJson_string('entity', *UUID)


mkPathField(*Path) = ipcJson_string('path', *Path)


mkAvuObject(*Field, *Name, *Value, *Unit) = ipcJson_object(
  *Field, 
  list(
    ipcJson_string('attribute', *Name), 
    ipcJson_string('value', *Value), 
    ipcJson_string('unit', *Unit) ) )


_getAmqpEntityType(*ItemType) =
  match *ItemType with
    | '-C' => COLLECTION_TYPE
    | '-d' => DATA_OBJECT_TYPE
    | '-R' => RESOURCE_TYPE
    | '-u' => USER_TYPE


sendCollectionAdd(*Collection, *Path) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Collection),
      mkPathField(*Path) ) )
  in sendMsg(COLLECTION_TYPE ++ '.add', *msg)


sendDataObjectOpen(*Data) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Data),
      mkPathField($objPath),
      ipcJson_number('size', double($dataSize)),
      ipcJson_string('timestamp', getTimestamp()) ) )
  in sendMsg(DATA_OBJECT_TYPE ++ '.open', *msg)


_ipc_sendDataObjectAdd(
  *AuthorName, *AuthorZone, *Data, *Path, *OwnerName, *OwnerZone, *Size, *Type
) {
  *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField(*AuthorName, *AuthorZone),
      mkEntityField(*Data),
      mkPathField(*Path),
      mkUserObject('creator', *OwnerName, *OwnerZone),
      ipcJson_number('size', double(*Size)),
      ipcJson_string('type', *Type) ) );

  sendMsg(DATA_OBJECT_TYPE ++ '.add', *msg);
}


# Publish a data-object.mod message to AMQP exchange
_ipc_sendDataObjectMod(
  *AuthorName, *AuthorZone, *Object, *Path, *OwnerName, *OwnerZone, *Size, *Type
) {
  *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField(*AuthorName, *AuthorZone),
      mkEntityField(*Object),
      mkUserObject('creator', *OwnerName, *OwnerZone),
      ipcJson_number('size', double(*Size)),
      ipcJson_string('type', *Type) ) );

  sendMsg(DATA_OBJECT_TYPE ++ '.mod', *msg);
}


sendCollectionInheritModified(*Recursive, *Inherit, *Collection) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Collection),
      ipcJson_boolean('recursive', *Recursive),
      ipcJson_boolean('inherit', *Inherit) ) )
  in sendMsg(COLLECTION_TYPE ++ '.acl.mod', *msg)


sendCollectionAclModified(*Recursive, *AccessLevel, *Username, *Zone, *Collection) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Collection),
      ipcJson_boolean('recursive', *Recursive),
      ipcJson_string('permission', *AccessLevel),
      mkUserObject('user', *Username, *Zone) ) )
  in sendMsg(COLLECTION_TYPE ++ '.acl.mod', *msg)


sendCollectionAccessModified(*Recursive, *AccessLevel, *Username, *Zone, *Collection) {
  if (*AccessLevel == 'inherit') {
    sendCollectionInheritModified(*Recursive, true, *Collection);
  }
  else if (*AccessLevel == 'noinherit') {
    sendCollectionInheritModified(*Recursive, false, *Collection);
  }
  else {
    sendCollectionAclModified(*Recursive, *AccessLevel, *Username, *Zone, *Collection);
  }
}


sendDataObjectAclModified(*AccessLevel, *Username, *Zone, *Data) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Data),
      ipcJson_string('permission', *AccessLevel),
      mkUserObject('user', *Username, *Zone) ) )
  in sendMsg(DATA_OBJECT_TYPE ++ '.acl.mod', *msg)


# Publish a data-object.sys-metadata.mod message to AMQP exchange
_ipc_sendDataObjectMetadataModified(*AuthorName, *AuthorZone, *Data) {
  *msg = ipcJson_document(list(_ipc_mkAuthorField(*AuthorName, *AuthorZone), mkEntityField(*Data)));
  sendMsg(DATA_OBJECT_TYPE ++ '.sys-metadata.mod', *msg);
}


sendEntityMove(*EntityType, *Entity, *OldPath, *NewPath) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Entity),
      ipcJson_string('old-path', *OldPath),
      ipcJson_string('new-path', *NewPath) ) )
  in sendMsg(*EntityType ++ '.mv', *msg)


sendEntityRemove(*EntityType, *Entity, *Path) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Entity),
      ipcJson_string('path', *Path) ) )
  in sendMsg(*EntityType ++ '.rm', *msg)


sendAvuMod(*ItemType, *Item, *OldName, *OldValue, *OldUnit, *NewName, *NewValue, *NewUnit) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Item),
      mkAvuObject('old-metadatum', *OldName, *OldValue, *OldUnit),
      mkAvuObject('new-metadatum', *NewName, *NewValue, *NewUnit) ) )
  in sendMsg(_getAmqpEntityType(*ItemType) ++ '.metadata.mod', *msg)


sendAvuSet(*Option, *ItemType, *Item, *AName, *AValue, *AUnit) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Item),
      mkAvuObject('metadatum', *AName, *AValue, *AUnit) ) )
  in sendMsg(_getAmqpEntityType(*ItemType) ++ '.metadata.' ++ *Option, *msg)


sendAvuMultiset(*ItemName, *AName, *AValue, *AUnit) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      ipcJson_string('pattern', *ItemName),
      mkAvuObject('metadatum', *AName, *AValue, *AUnit) ) )
  in sendMsg(DATA_OBJECT_TYPE ++ '.metadata.addw', *msg)


sendAvuMultiremove(*ItemType, *Item, *AName, *AValue, *AUnit) =
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      mkEntityField(*Item),
      ipcJson_string('attribute-pattern', *AName),
      ipcJson_string('value-pattern', *AValue),
      ipcJson_string('unit-pattern', *AUnit) ) )
  in sendMsg(_getAmqpEntityType(*ItemType) ++ '.metadata.rmw', *msg)


sendAvuCopy(*SourceItemType, *TargetItemType, *Source, *Target) =
  let *srcType = _getAmqpEntityType(*SourceItemType) in
  let *msg = ipcJson_document(
    list(
      _ipc_mkAuthorField($userNameClient, $rodsZoneClient),
      ipcJson_string('source', *Source),
      ipcJson_string('source-type', *srcType),
      ipcJson_string('destination', *Target) ) )
  in sendMsg(_getAmqpEntityType(*TargetItemType) ++ '.metadata.cp', *msg)


resolveAdminPerm(*Item) = if *Item like regex '^/[^/]*(/[^/]*)?$' then 'write' else 'own'


setAdminGroupPerm(*Item) {
  msiSetACL('default', resolveAdminPerm(*Item), 'rodsadmin', *Item);
}


canModProtectedAVU(*User) {
  *canMod = false;
  *res = SELECT USER_ID WHERE USER_NAME = *User AND USER_TYPE = 'rodsadmin';
  foreach (*record in *res) {
    *canMod = true;
    break;
  }
  *canMod;
}


# Gets the original unit for an AVU modification. The argument that is used for 
# the original unit in the AVU modification may contain the original unit or, if 
# the unit was empty in the original AVU then this argument may contain the 
# first of the new AVU settings instead. We can distinguish this case from the 
# others by the presence of a prefix in the value. The prefix is always a single 
# character followed by a colon.
#
getOrigUnit(*candidate) =
  if strlen(*candidate) < 2 then *candidate
  else if substr(*candidate, 1, 1) != ':' then *candidate
  else '';


# Gets the new AVU setting from a list of candidates. New AVU settings are 
# passed in an arbitrary order and the type of AVU setting is identified by a 
# prefix. This function looks for values matching the given prefix. If multiple
#  matching values are found then the last one wins.
#
getNewAVUSetting(*orig, *prefix, *candidates) {
  *setting = *orig
  foreach (*candidate in *candidates) {
    if (strlen(*candidate) >= strlen(*prefix)) {
      if (substr(*candidate, 0, strlen(*prefix)) == *prefix) {
        *setting = substr(*candidate, 2, strlen(*candidate));
      }
    }
  }
  *setting;
}


# Determines whether or not the string in the first argument starts with the 
# string in the second argument.
#
startsWith(*str, *prefix) =
  if strlen(*str) < strlen(*prefix) then false
  else if substr(*str, 0, strlen(*prefix)) != *prefix then false
  else true;


# Removes a prefix from a string.
#
removePrefix(*orig, *prefixes) {
  *result = *orig
  foreach (*prefix in *prefixes) {
    if (startsWith(*orig, *prefix)) {
      *result = substr(*orig, strlen(*prefix), strlen(*orig));
      break;
    }
  }
  *result;
}


# Compute the checksum of of a given replica of a given data object
_ipc_chksumRepl(*Object, *ReplNum) {
  msiAddKeyValToMspStr('forceChksum', '', *opts);
  msiAddKeyValToMspStr('replNum', str(*ReplNum), *opts);
  msiDataObjChksum(*Object, *opts, *_);
}


# Indicates whether or not an AVU is protected
avuProtected(*ItemType, *ItemName, *Attribute) {
  if (startsWith(*Attribute, 'ipc')) {
    *Attribute != 'ipc_UUID' || retrieveUUID(*ItemType, *ItemName) != '';
  } else {
    false;
  }
}


# Verifies that an attribute can be modified. If it can't it fails and sends an
# error message to the caller.
ensureAVUEditable(*Editor, *ItemType, *ItemName, *A, *V, *U) {
  if (avuProtected(*ItemType, *ItemName, *A) && !canModProtectedAVU(*Editor)) {
    cut;
    failmsg(-830000, 'CYVERSE ERROR:  attempt to alter protected AVU <*A, *V, *U>');
  }
}


# If an AVU is not protected, it sets the AVU to the given item
setAVUIfUnprotected(*ItemType, *ItemName, *A, *V, *U) {
  if (!avuProtected(*ItemType, *ItemName, *A)) {
    msiModAVUMetadata(*ItemType, *ItemName, 'set', *A, *V, *U);
  }
}


# Copies the unprotected AVUs from a given collection to the given item.
cpUnprotectedCollAVUs(*Coll, *TargetType, *TargetName) =
  foreach (*avu in SELECT META_COLL_ATTR_NAME, META_COLL_ATTR_VALUE, META_COLL_ATTR_UNITS
                     WHERE COLL_NAME == *Coll) {
    setAVUIfUnprotected(*TargetType, *TargetName, *avu.META_COLL_ATTR_NAME,
                        *avu.META_COLL_ATTR_VALUE, *avu.META_COLL_ATTR_UNITS);
  }


# Copies the unprotected AVUs from a given data object to the given item.
cpUnprotectedDataObjAVUs(*ObjPath, *TargetType, *TargetName) {
  msiSplitPath(*ObjPath, *parentColl, *objName);
  foreach (*avu in SELECT META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE, META_DATA_ATTR_UNITS
                     WHERE COLL_NAME == *parentColl AND DATA_NAME == *objName) {
    setAVUIfUnprotected(*TargetType, *TargetName, *avu.META_DATA_ATTR_NAME,
                        *avu.META_DATA_ATTR_VALUE, *avu.META_DATA_ATTR_UNITS);
  }
}


# Copies the unprotected AVUs from a given resource to the given item.
cpUnprotectedRescAVUs(*Resc, *TargetType, *TargetName) =
  foreach (*avu in SELECT META_RESC_ATTR_NAME, META_RESC_ATTR_VALUE, META_RESC_ATTR_UNITS
                     WHERE RESC_NAME == *Resc) {
    setAVUIfUnprotected(*TargetType, *TargetName, *avu.META_RESC_ATTR_NAME,
                        *avu.META_RESC_ATTR_VALUE, *avu.META_RESC_ATTR_UNITS);
  }


# Copies the unprotected AVUs from a given user to the given item.
cpUnprotectedUserAVUs(*User, *TargetType, *TargetName) =
  foreach (*avu in SELECT META_USER_ATTR_NAME, META_USER_ATTR_VALUE, META_USER_ATTR_UNITS
                     WHERE USER_NAME == *User) {
    setAVUIfUnprotected(*TargetType, *TargetName, *avu.META_RESC_ATTR_NAME,
                        *avu.META_RESC_ATTR_VALUE, *avu.META_RESC_ATTR_UNITS);
  }


# Create a user for a Data Store service
ipc_acCreateUser {
  msiCreateUser ::: msiRollback;
  ipc_acCreateUserZoneCollections  :::  msiRollback;
  msiAddUserToGroup("public")  :::  msiRollback;
  msiCommit;
}

ipc_acCreateUserZoneCollections {
  acCreateCollByAdmin("/" ++ ipc_ZONE ++ "/home", $otherUserName);
  acCreateCollByAdmin("/" ++ ipc_ZONE ++ "/trash/home", $otherUserName);
}


# Refuse SSL connections
#
ipc_acPreConnect(*OUT) { *OUT = 'CS_NEG_REFUSE'; }


# Use default threading setting
#
ipc_acSetNumThreads { msiSetNumThreads('default', 'default', 'default'); }


# Set maximum number of rule engine processes
#
ipc_acSetReServerNumProc { msiSetReServerNumProc(str(ipc_MAX_NUM_RE_PROCS)); }


# This rule sets the rodsadin group permission of a collection when a collection
# is created by an administrative means, i.e. iadmin mkuser. It also pushes a
# collection.add message into the irods exchange.
#
ipc_acCreateCollByAdmin(*ParColl, *ChildColl) {
  *coll = '*ParColl/*ChildColl';
  *perm = resolveAdminPerm(*coll);
  msiSetACL('default', 'admin:*perm', 'rodsadmin', *coll);
}


ipc_archive_acCreateCollByAdmin(*ParColl, *ChildColl) {
  *coll = '*ParColl/*ChildColl';
  sendCollectionAdd(assignUUID('-C', *coll), *coll);
}


# This rule pushes a collection.rm message into the irods exchange.
#
ipc_acDeleteCollByAdmin(*ParColl, *ChildColl) {
  *path = '*ParColl/*ChildColl';
  *uuid = retrieveCollectionUUID(*path);
  if (*uuid != '') {
    sendEntityRemove(COLLECTION_TYPE, *uuid, *path);
  }
}


# This rule prevents the user from removing rodsadmin's ownership from an ACL 
# unless the user is of type rodsadmin.
#
ipc_acPreProcForModifyAccessControl(*RecursiveFlag, *AccessLevel, *UserName, *Zone, *Path) {
  if (*UserName == 'rodsadmin') {
    if (!(*AccessLevel like 'admin:*') && *AccessLevel != resolveAdminPerm(*Path)) {
      cut;
      failmsg(-830000, 'CYVERSE ERROR:  attempt to alter admin user permission.');
    }
  }
}


# This rule makes the admin owner of any created collection.  This rule is not 
# applied to collections created when a TAR file is expanded. (i.e. ibun -x)
#
ipc_acPostProcForCollCreate {
  setAdminGroupPerm($collName);
}


# This rule ensures that archival collections are given a UUID and an AMQP
# message is published indicating the collection is created.
#
ipc_archive_acPostProcForCollCreate {
  sendCollectionAdd(assignUUID('-C', $collName), $collName);
}


ipc_acPostProcForOpen {
  *uuid = retrieveDataUUID($objPath);
  if (*uuid != '') { sendDataObjectOpen(*uuid); }
}


ipc_acPreprocForRmColl { temporaryStorage.'$collName' = retrieveCollectionUUID($collName); }


ipc_acPostProcForRmColl {
  *uuid = temporaryStorage.'$collName';
  if (*uuid != '') { sendEntityRemove(COLLECTION_TYPE, *uuid, $collName); }
}


ipc_acDataDeletePolicy { temporaryStorage.'$objPath' = retrieveDataUUID($objPath); }


ipc_acPostProcForDelete {
  *uuid = temporaryStorage.'$objPath';
  if (*uuid != '') { sendEntityRemove(DATA_OBJECT_TYPE, *uuid, $objPath); }
}


# This sends a collection or data-object ACL modification message for the 
# updated object.
#
ipc_acPostProcForModifyAccessControl(*RecursiveFlag, *AccessLevel, *UserName, *Zone, *Path) {
  *level = removePrefix(*AccessLevel, list('admin:'));
  *type = ipc_getEntityType(*Path);
  *uuid = retrieveUUID(*type, *Path);
  if (*uuid != '') {
    if (*type == '-C') {
      sendCollectionAccessModified(bool(*RecursiveFlag), *level, *UserName, *Zone, *uuid);
    } else if (*type == '-d') {
      sendDataObjectAclModified(*level, *UserName, *Zone, *uuid);
    }
  }
}


# This rule schedules a rename entry job for the data object or collection being
# renamed.
#
ipc_acPostProcForObjRename(*SrcEntity, *DestEntity) {
  *type = ipc_getEntityType(*DestEntity);
  *uuid = retrieveUUID(*type, *DestEntity);

  if (*uuid != '') {
    if (*type == '-C') {
      sendEntityMove(COLLECTION_TYPE, *uuid, *SrcEntity, *DestEntity);
    } else if (*type == '-d') {
      sendEntityMove(DATA_OBJECT_TYPE, *uuid, *SrcEntity, *DestEntity);
    }
  } else {
    if (*type == '-C') {
      *err = errormsg(sendCollectionAdd(assignUUID('-C', *DestEntity), *DestEntity), *msg);
      if (*err < 0) { writeLine('serverLog', *msg); }
    } else if (*type == '-d') {
      *uuid = assignUUID('-d', *DestEntity);
      msiSplitPath(*DestEntity, *collPath, *dataName);

      *sent = false;

      foreach (*res in 
        select order_asc(DATA_SIZE), DATA_TYPE_NAME, DATA_OWNER_NAME, DATA_OWNER_ZONE
        where COLL_NAME = '*collPath' and DATA_NAME = '*dataName'
      ) {
        if (!*sent) {
          *err = errormsg(
            _ipc_sendDataObjectAdd(
              $userNameClient, 
              $rodsZoneClient, 
              *uuid, 
              *DestEntity, 
              *res.DATA_OWNER_NAME, 
              *res.DATA_OWNER_ZONE, 
              *res.DATA_SIZE, 
              *res.DATA_TYPE_NAME),
            *msg);

          if (*err < 0) { writeLine('serverLog', *msg); }
          *sent = true;
        }
      }
    }
  }
}


# This rule checks that AVU being modified isn't a protected one.
ipc_acPreProcForModifyAVUMetadata(*Option, *ItemType, *ItemName, *AName, *AValue, *New1, *New2,
                                  *New3, *New4) {
  *newArgs = list(*New1, *New2, *New3, *New4);

  # Determine the original unit and the new AVU settings.
  *origUnit = getOrigUnit(*New1);
  *newName = getNewAVUSetting(*AName, 'n:', *newArgs);
  *newValue = getNewAVUSetting(*AValue, 'v:', *newArgs);
  *newUnit = getNewAVUSetting(*origUnit, 'u:', *newArgs);

  ensureAVUEditable($userNameClient, *ItemType, *ItemName, *AName, *AValue, *origUnit);
  ensureAVUEditable($userNameClient, *ItemType, *ItemName, *newName, *newValue, *newUnit);
}


# This rule checks that AVU being added, set or removed isn't a protected one.
# Only rodsadmin users are allowed to add, remove or update protected AVUs.
ipc_acPreProcForModifyAVUMetadata(*Option, *ItemType, *ItemName, *AName, *AValue, *AUnit) {
  if (*Option == 'add' || *Option == 'addw') {
    ensureAVUEditable($userNameProxy, *ItemType, *ItemName, *AName, *AValue, *AUnit);
  } else if (*Option == 'set') {
    if (*ItemType == '-C') {
      *query =
        SELECT META_COLL_ATTR_ID WHERE COLL_NAME == *ItemName AND META_COLL_ATTR_NAME == *AName;
    } else if (*ItemType == '-d') {
      msiSplitPath(*ItemName, *collPath, *dataName);

      *query =
        SELECT META_DATA_ATTR_ID
        WHERE COLL_NAME == *collPath AND DATA_NAME == *dataName AND META_DATA_ATTR_NAME == *AName;
    } else if (*ItemType == '-R') {
      *query =
        SELECT META_RESC_ATTR_ID WHERE RESC_NAME == *ItemName AND META_RESC_ATTR_NAME == *AName;
    } else if (*ItemType == '-u') {
      *query =
        SELECT META_USER_ATTR_ID WHERE USER_NAME == *ItemName AND META_USER_ATTR_NAME == *AName;
    } else {
      writeLine('serverLog', 'unknown imeta item type "*ItemType"');
      fail;
    }

    *exists = false;

    foreach (*record in *query) {
      *exists = true;
      break;
    }

    *authenticatee = if *exists then $userNameClient else $userNameProxy;
    ensureAVUEditable(*authenticatee, *ItemType, *ItemName, *AName, *AValue, *AUnit);
  } else if (*Option == 'rm' || *Option == 'rmw') {
    ensureAVUEditable($userNameClient, *ItemType, *ItemName, *AName, *AValue, *AUnit);
  } else if (*Option != 'adda') {
    writeLine('serverLog', 'unknown imeta option "*Option"');
  }
}


# This rule ensures that only the non-protected AVUs are copied from one item to
#  another.
ipc_acPreProcForModifyAVUMetadata(*Option, *SourceItemType, *TargetItemType, *SourceItemName,
                                  *TargetItemName) {
  if (!canModProtectedAVU($userNameClient)) {
    if (*SourceItemType == '-C') {
      cpUnprotectedCollAVUs(*SourceItemName, *TargetItemType, *TargetItemName);
    } else if (*SourceItemType == '-d') {
      cpUnprotectedDataObjAVUs(*SourceItemName, *TargetItemType, *TargetItemName);
    } else if (*SourceItemType == '-R') {
      cpUnprotectedRescAVUs(*SourceItemName, *TargetItemType, *TargetItemName);
    } else if (*SourceItemType == '-u') {
      cpUnprotectedUserAVUs(*SourceItemName, *TargetItemType, *TargetItemName);
    }

    # fail to prevent iRODS from also copying the protected metadata
    cut;
    failmsg(0, 'CYVERSE SUCCESS:  Successfully copied the unprotected metadata.');
  }
}


# This rule sends a message indicating that an AVU was modified.
#
ipc_acPostProcForModifyAVUMetadata(*Option, *ItemType, *ItemName, *AName, *AValue, *new1,
                                   *new2, *new3, *new4) {
  *uuid = retrieveUUID(*ItemType, *ItemName);
  if (*uuid != '') {
    *newArgs = list(*new1, *new2, *new3, *new4);

    # Determine the original unit and the new AVU settings.
    *origUnit = getOrigUnit(*new1);
    *newName = getNewAVUSetting(*AName, 'n:', *newArgs);
    *newValue = getNewAVUSetting(*AValue, 'v:', *newArgs);
    *newUnit = getNewAVUSetting(*origUnit, 'u:', *newArgs);

    # Send AVU modified message.
    sendAvuMod(*ItemType, *uuid, *AName, *AValue, *origUnit, *newName, *newValue, *newUnit);
  }
}


# This rule sends one of the AVU metadata set messages, depending on which 
# subcommand was used.
#
ipc_acPostProcForModifyAVUMetadata(*Option, *ItemType, *ItemName, *AName, *AValue, *AUnit) {
  if (*AName != 'ipc_UUID') {
    if (contains(*Option, list('add', 'adda', 'rm', 'set'))) {
      *uuid = retrieveUUID(*ItemType, *ItemName);
      if (*uuid != '') {
        sendAvuSet(*Option, *ItemType, *uuid, *AName, *AValue, *AUnit);
      }
    } else if (*Option == 'addw') {
      sendAvuMultiset(*ItemName, *AName, *AValue, *AUnit);
    } else if (*Option == 'rmw') {
      *uuid = retrieveUUID(*ItemType, *ItemName);
      if (*uuid != '') {
        sendAvuMultiremove(*ItemType, *uuid, *AName, *AValue, *AUnit)
      }
    }
  }
}


# This rules sends an AVU metadata copy message.
#
ipc_acPostProcForModifyAVUMetadata(*Option, *SourceItemType, *TargetItemType, *SourceItemName,
                                   *TargetItemName) {
  *source = match *SourceItemType with
              | '-C' => retrieveCollectionUUID(*SourceItemName)
              | '-d' => retrieveDataUUID(*SourceItemName)
              | '-R' => *SourceItemName
              | '-u' => *SourceItemName;

  *target = match *TargetItemType with
              | '-C' => retrieveCollectionUUID(*TargetItemName)
              | '-d' => retrieveDataUUID(*TargetItemName)
              | '-R' => *TargetItemName
              | '-u' => *TargetItemName;

  if (*source != '' && *target != '') {
    sendAvuCopy(*SourceItemType, *TargetItemType, *source, *target);
  }
}


# Whenever a large file is uploaded, recheck the free space on the storage
# resource server where the file was written.
#
ipc_acPostProcForParallelTransferReceived(*LeafResource) {
  msi_update_unixfilesystem_resource_free_space(*LeafResource);
}


# XXX - Because of https://github.com/irods/irods/issues/5540
# ipc_dataObjCreated_default(*User, *Zone, *DATA_OBJ_INFO) {
#   *err = errormsg(_ipc_chksumRepl(*DATA_OBJ_INFO.logical_path, 0), *msg);
#   if (*err < 0) { writeLine('serverLog', *msg); }
#
#   *err = errormsg(setAdminGroupPerm(*DATA_OBJ_INFO.logical_path), *msg);
#   if (*err < 0) { writeLine('serverLog', *msg); }
#
#   *uuid = assignUUID('-d', *DATA_OBJ_INFO.logical_path);
#        
#   *err = errormsg(
#     _ipc_sendDataObjectAdd(
#       *User,
#       *Zone,
#       *uuid,
#       *DATA_OBJ_INFO.logical_path,
#       *DATA_OBJ_INFO.data_owner_name,
#       *DATA_OBJ_INFO.data_owner_zone,
#       *DATA_OBJ_INFO.data_size,
#       *DATA_OBJ_INFO.data_type),
#     *msg);
#   if (*err < 0) { writeLine('serverLog', *msg); }
# }
ipc_dataObjCreated_default(*User, *Zone, *DATA_OBJ_INFO, *Step) {
  if (*Step != 'FINISH') {
    *err = errormsg(setAdminGroupPerm(*DATA_OBJ_INFO.logical_path), *msg);
    if (*err < 0) { writeLine('serverLog', *msg); }

    assignUUID('-d', *DATA_OBJ_INFO.logical_path);
  }

  if (*Step != 'START') {
    *err = errormsg(_ipc_chksumRepl(*DATA_OBJ_INFO.logical_path, 0), *msg);
    if (*err < 0) { writeLine('serverLog', *msg); }
   
    *uuid = retrieveUUID('-d', *DATA_OBJ_INFO.logical_path);

    if (*uuid != '') {     
      *err = errormsg(
        _ipc_sendDataObjectAdd(
          *User,
          *Zone,
          *uuid,
          *DATA_OBJ_INFO.logical_path,
          *DATA_OBJ_INFO.data_owner_name,
          *DATA_OBJ_INFO.data_owner_zone,
          *DATA_OBJ_INFO.data_size,
          *DATA_OBJ_INFO.data_type),
        *msg);
      if (*err < 0) { writeLine('serverLog', *msg); }
    }
  }
}
# XXX - ^^^


# XXX - Because of https://github.com/irods/irods/issues/5540
# ipc_dataObjCreated_staging(*User, *Zone, *DATA_OBJ_INFO) {
#   *err = errormsg(_ipc_chksumRepl(*DATA_OBJ_INFO.logical_path, 0), *msg);
#   if (*err < 0) { writeLine('serverLog', *msg); }
#
#   *err = errormsg(setAdminGroupPerm(*DATA_OBJ_INFO.logical_path), *msg);
#   if (*err < 0) { writeLine('serverLog', *msg); }
# }
ipc_dataObjCreated_staging(*User, *Zone, *DATA_OBJ_INFO, *Step) {
  if (*Step != 'FINISH') {
    *err = errormsg(setAdminGroupPerm(*DATA_OBJ_INFO.logical_path), *msg);
    if (*err < 0) { writeLine('serverLog', *msg); }
  }

  if (*Step != 'START') {
    *err = errormsg(_ipc_chksumRepl(*DATA_OBJ_INFO.logical_path, 0), *msg);
    if (*err < 0) { writeLine('serverLog', *msg); }
  }
}
# XXX - ^^^


ipc_dataObjModified_default(*User, *Zone, *DATA_OBJ_INFO) {
  *uuid = retrieveUUID('-d', *DATA_OBJ_INFO.logical_path);
    
  if (*uuid != '') {
    _ipc_sendDataObjectMod(
      *User,
      *Zone,
      *uuid,
      *DATA_OBJ_INFO.logical_path,
      *DATA_OBJ_INFO.data_owner_name,
      *DATA_OBJ_INFO.data_owner_zone,
      *DATA_OBJ_INFO.data_size,
      *DATA_OBJ_INFO.data_type);
  }
}


# This rule sends a system metadata modified status message.
#
ipc_dataObjMetadataModified(*User, *Zone, *Object) {
  *uuid = retrieveDataUUID(*Object);
  if (*uuid != '') {
    _ipc_sendDataObjectMetadataModified(*User, *Zone, *uuid);
  }
}
