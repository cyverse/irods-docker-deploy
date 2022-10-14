FROM irods-docker-deploy-ubuntu2004

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
      apt-get install --no-install-recommends -y --quiet=2 locales && \
      rm -rf /var/lib/apt/lists/*

RUN locale-gen en_US.UTF-8

ARG LC_COLLATE=C
RUN apt-get update && \
      apt-get install --yes --quiet=2 postgresql postgresql-client python3-psycopg2 && \
      rm -rf /var/lib/apt/lists/*

COPY postgresql/config/* /etc/postgresql/12/main/
RUN chown postgres:postgres /etc/postgresql/12/main/*

COPY postgresql/scripts/*.sql* /tmp/
COPY postgresql/scripts/config.sh /tmp/config

USER postgres

ARG DB_NAME=ICAT
ARG DB_PASSWORD=testpassword
ARG DB_USER=irods
ARG DBMS_PORT=5432
ARG IRODS_RESOURCES=demoResc:rs:/var/lib/irods/Vault
ARG IRODS_ZONE_NAME=tempZone
ARG IRODS_ADMIN_PASSWORD=rods
ARG IRODS_ADMIN_USER=rods
ARG IRODS_BISQUE_ADMIN_PASSWORD=bisque_admin
ARG IRODS_BISQUE_ADMIN_USER=bisque_admin
RUN /tmp/config

USER root

COPY postgresql/service.sh /
RUN chmod a+rx /service.sh

CMD [ "/service.sh" ]
