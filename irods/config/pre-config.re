# This is a stub configuration

_ipc_updateQuotaUsage {}
ipc_rescheduleQuotaUsageUpdate {
  delay('<PLUSET>0s</PLUSET><EF>1h REPEAT FOR EVER</EF>') {_ipc_updateQuotaUsage};
}

_ipc_determineAllStorageFreeSpace {}
ipc_rescheduleStorageFreeSpaceDetermination {
  delay('<PLUSET>0s</PLUSET><EF>1h REPEAT FOR EVER</EF>') {_ipc_determineAllStorageFreeSpace};
}

_ipc_rmTrash {}
ipc_rescheduleTrashRemoval {
  delay('<PLUSET>0s</PLUSET><EF>1h REPEAT FOR EVER</EF>') {_ipc_rmTrash};
}


acCreateUser {
  on ($otherUserType == 'ucsb-irods-service') {
    msiCreateUser ::: msiRollback;
    msiCommit;
  }
}
