# VERSION 12
#
# bisque.re
#
# BisQue related rules

@include 'bisque-env'

_bisque_logMsg(*Msg) {
  writeLine('serverLog', 'BISQUE: *Msg');
}

# Add a call to this rule from inside the acPostProcForCollCreate PEP.
bisque_acPostProcForCollCreate {
  if ($userNameClient != bisque_IRODS_ADMIN_USER) {
    ipc_giveAccessColl(bisque_IRODS_ADMIN_USER, 'write', $collName);
  }
}

bisque_dataObjCreated(*User, *_, *DATA_OBJ_INFO) {
  if (*User != bisque_IRODS_ADMIN_USER) {
    ipc_giveAccessObj(bisque_IRODS_ADMIN_USER, 'write', *DATA_OBJ_INFO.logical_path);
  }
}

# a new bisque user is created
bisque_acCreateUser {
  # set usergroup
  msiAddUserToGroup("bisque_group")
  # set ACL
  msiSetACL('default', 'write', bisque_IRODS_ADMIN_USER, "/" ++ ipc_ZONE ++ "/home/" ++ $otherUserName)
  msiSetACL('default', 'write', bisque_IRODS_ADMIN_USER, "/" ++ ipc_ZONE ++ "/trash/home/" ++ $otherUserName)
  msiSetACL('recursive', 'inherit', bisque_IRODS_ADMIN_USER, "/" ++ ipc_ZONE ++ "/home/" ++ $otherUserName)
  msiSetACL('recursive', 'inherit', bisque_IRODS_ADMIN_USER, "/" ++ ipc_ZONE ++ "/trash/home/" ++ $otherUserName)
}
