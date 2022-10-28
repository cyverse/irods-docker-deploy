FROM rabbitmq:3.9.24-management

ARG RABBITMQ_PORT=5672
ARG RABBITMQ_MANAGEMENT_PORT=15672
ARG RABBITMQ_ADMIN_USER=admin
ARG RABBITMQ_ADMIN_PASSWORD=admin_password
ARG RABBITMQ_IRODS_USER=irods
ARG RABBITMQ_IRODS_PASSWORD=
ARG RABBITMQ_IRODS_EXCHANGE=irods

COPY rabbitmq/scripts/gen_password.py /usr/local/bin/

COPY rabbitmq/scripts/definitions.json.template /tmp/definitions.json.template
COPY rabbitmq/scripts/prep-definitions-json.sh /usr/local/bin/
RUN prep-definitions-json.sh

COPY rabbitmq/scripts/rabbitmq.conf.template /tmp/rabbitmq.conf.template
COPY rabbitmq/scripts/prep-rabbitmq-conf.sh /usr/local/bin/
RUN prep-rabbitmq-conf.sh
