# VERSION 4
#
# These are the rules related to UUIDs


ipc_uuidGenerate {
  *status = errorcode(msiExecCmd("generateuuid.sh", "", "null", "null", "null", *out));
  if (*status == 0) {
    msiGetStdoutInExecCmdOut(*out, *uuid);
    trimr(*uuid, "\n");
  } else {
    writeLine("serverLog", "failed to generate UUID");
    fail;
  }
}
