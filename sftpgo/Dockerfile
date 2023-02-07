FROM cyverse/sftpgo

ARG WEBDAV_PORT=8080
ARG SFTP_PORT=2022
ARG SFTPGO_ADMIN_UI_PORT=8022
ARG SFTPGO_VAULT="/srv/sftpgo"
ARG SFTPGO_HOME_PATH="/srv/sftpgo/data"
ENV TZ="America/Los_Angeles"

USER root
RUN apt-get update && \
      apt-get install --no-install-recommends -y curl && \
      rm -rf /var/lib/apt/lists/*

# extract sftpgo-auth-irods
RUN mkdir -p /tmp/sftpgo-auth-irods
RUN curl -L https://github.com/cyverse/sftpgo-auth-irods/releases/download/v0.1.5/sftpgo-auth-irods-v0.1.5-linux-amd64.tar.gz --output /tmp/sftpgo-auth-irods/sftpgo-auth-irods.tar.gz
RUN tar zxvf /tmp/sftpgo-auth-irods/sftpgo-auth-irods.tar.gz -C /tmp/sftpgo-auth-irods && cp /tmp/sftpgo-auth-irods/sftpgo-auth-irods /usr/local/bin && rm -rf /tmp/sftpgo-auth-irods

COPY sftpgo/scripts/sftpgo.json.template /tmp/sftpgo.json.template
COPY sftpgo/scripts/prep-config-json.sh /usr/local/bin/
RUN prep-config-json.sh

COPY sftpgo/host_keys/* /var/lib/sftpgo/
COPY sftpgo/config/blacklist.json /var/lib/sftpgo/
RUN chmod -R a+r /var/lib/sftpgo

# set setuid/setgid bits, since users may want to bind system ports (i.e., 22)
RUN chmod ug+s /usr/local/bin/sftpgo
RUN chmod ug+s /usr/local/bin/sftpgo-auth-irods

USER 1000:1000
WORKDIR /var/lib/sftpgo

CMD ["sftpgo", "serve"]