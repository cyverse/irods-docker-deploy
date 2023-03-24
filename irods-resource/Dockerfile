FROM irods-docker-deploy-centos7

### Install ansible support
RUN yum --assumeyes install \
      jq python-pip python-requests python-virtualenv \
      ### Install software needed build image
      python3 \
      curl

### Install iRODS 
ADD https://packages.irods.org/renci-irods.yum.repo /etc/yum.repos.d/renci-irods.yum.repo

RUN rpm --import https://packages.irods.org/irods-signing-key.asc && \
      yum --assumeyes install \
      irods-server-4.2.8
### Install irods-rule-async-exec-cmd
ARG IRODS_RULE_ASYNC_EXEC_CMD_VER=v0.2.13
RUN mkdir -p /tmp/irods-rule-async-exec-cmd
RUN curl -L https://github.com/cyverse/irods-rule-async-exec-cmd/releases/download/${IRODS_RULE_ASYNC_EXEC_CMD_VER}/irods-rule-async-exec-cmd-svc-${IRODS_RULE_ASYNC_EXEC_CMD_VER}-linux-amd64.tar.gz --output /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd-svc.tar.gz
RUN curl -L https://github.com/cyverse/irods-rule-async-exec-cmd/releases/download/${IRODS_RULE_ASYNC_EXEC_CMD_VER}/irods-rule-async-exec-cmd-${IRODS_RULE_ASYNC_EXEC_CMD_VER}-linux-amd64.tar.gz --output /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd.tar.gz
RUN tar zxvf /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd-svc.tar.gz -C /tmp/irods-rule-async-exec-cmd && cp /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd-svc /usr/local/bin
RUN tar zxvf /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd.tar.gz -C /tmp/irods-rule-async-exec-cmd && cp /tmp/irods-rule-async-exec-cmd/irods-rule-async-exec-cmd /usr/local/bin
RUN rm -rf /tmp/irods-rule-async-exec-cmd
RUN mkdir -p /var/lib/irods_rule_async_exec_cmd
ARG IRODS_CONTROL_PLANE_KEY=TEMPORARY__32byte_ctrl_plane_key
ARG IRODS_CONTROL_PLANE_PORT=1248
ARG IRODS_DEFAULT_RESOURCE=demoResc
ARG IRODS_FIRST_EPHEMERAL_PORT=20000
ARG IRODS_LAST_EPHEMERAL_PORT=20199
ARG IRODS_HOST=ies
ARG IRODS_RESOURCE_HOST=iresc
ARG IRODS_NEGOTIATION_KEY=TEMPORARY_32byte_negotiation_key
ARG IRODS_SCHEMA_VALIDATION=https://schemas.irods.org/configuration
ARG IRODS_SYSTEM_GROUP=irods
ARG IRODS_SYSTEM_USER=irods
ARG IRODS_ZONE_KEY=TEMPORARY_zone_key
ARG IRODS_ZONE_NAME=tempZone
ARG IRODS_ADMIN_PASSWORD=rods
ARG IRODS_ZONE_PORT=1247
ARG IRODS_ADMIN_USER=rods
ARG IRODS_MAX_NUM_RE_PROCS=4
ARG RABBITMQ_URL=
ARG RABBITMQ_IRODS_EXCHANGE=irods
ARG IRODS_BISQUE_ADMIN_USER=bisque_admin
ARG BISQUE_URL=
ARG BISQUE_ADMIN_USER=bisque_admin
ARG BISQUE_ADMIN_PASSWORD=
ARG BISQUE_IRODS_BASE_URL=
ARG BISQUE_IRODS_ROOT_PATH=/

### Configure iRODS
COPY irods-resource/scripts/config-irods.sh /tmp/config-irods
RUN /tmp/config-irods

### Create service script
COPY irods-resource/scripts/service.sh.template /tmp/
COPY irods-resource/scripts/prep-svc-script.sh /tmp/prep-svc-script
RUN /tmp/prep-svc-script

### Install iRODS Rules
COPY irods-resource/rules/ipc-custom.re /etc/irods/
COPY irods-resource/rules/ipc-services.re /etc/irods/
COPY irods-resource/rules/ipc-logic.re /etc/irods/
COPY irods-resource/rules/ipc-uuid.re /etc/irods/
COPY irods-resource/rules/ipc-json.re /etc/irods/
COPY irods-resource/rules/ipc-env.re.template /tmp/
COPY irods-resource/rules/bisque.re /etc/irods/
COPY irods-resource/rules/bisque-env.re.template /tmp/
COPY irods-resource/scripts/prep-re-script.sh /tmp/prep-re-script
RUN /tmp/prep-re-script

### Install scripts used by iRODS Rules
COPY irods-resource/rule_scripts/generateuuid.sh /var/lib/irods/msiExecCmd_bin/
COPY irods-resource/rule_scripts/setuuid.sh /var/lib/irods/msiExecCmd_bin/
RUN cp /usr/local/bin/irods-rule-async-exec-cmd /var/lib/irods/msiExecCmd_bin/

### Install scripts used by irods-rule-async-exec-cmd
COPY irods-resource/rule_async_exec_cmd/config.yml.template /tmp/rule_async_exec_cmd_config.yml.template
COPY irods-resource/scripts/prep-rule-async-exec-cmd-config.sh /tmp/prep-rule-async-exec-cmd-config
RUN /tmp/prep-rule-async-exec-cmd-config

CMD [ "/service.sh" ]