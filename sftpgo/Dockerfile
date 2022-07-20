FROM golang:1.18-bullseye as auth_builder

RUN mkdir -p /workspace
WORKDIR /workspace

RUN apt-get update && \
      apt-get install --no-install-recommends -y unzip make && \
      rm -rf /var/lib/apt/lists/*

RUN curl -L https://github.com/cyverse/sftpgo-auth-irods/archive/refs/heads/main.zip --output sftpgo-auth-irods.zip && \
      unzip sftpgo-auth-irods.zip && \
      mv sftpgo-auth-irods-main sftpgo-auth-irods

WORKDIR /workspace/sftpgo-auth-irods
RUN make

FROM cyverse/sftpgo

ARG SFTP_PORT=2022
ARG SFTPGO_ADMIN_UI_PORT=8080
ARG SFTPGO_VAULT="/srv/sftpgo"
ENV TZ="America/Los_Angeles"

COPY --from=auth_builder /workspace/sftpgo-auth-irods/bin/sftpgo-auth-irods /usr/local/bin/
COPY sftpgo/scripts/sftpgo.json.template /tmp/sftpgo.json.template
COPY sftpgo/scripts/prep-config-json.sh /usr/local/bin/
RUN prep-config-json.sh

COPY sftpgo/host_keys/* /var/lib/sftpgo/
COPY sftpgo/config/blacklist.json /var/lib/sftpgo/

# set setuid/setgid bits, since users may want to bind system ports (i.e., 22)
USER root
RUN chmod ug+s /usr/local/bin/sftpgo
RUN chmod ug+s /usr/local/bin/sftpgo-auth-irods
USER 1000:1000

CMD ["sftpgo", "serve"]