# irods-docker-deploy
Deploy iRODS using Docker


## Build
Update configuration parameters set in `config.inc`.

```bash
./build
```

## Create Volumes
Creates Docker volumes set in `config.inc`.
By default, it creates two volumes, `irods_volume` and `db_volume`.

```bash
./controller create_volumes
```

## Start

```bash
./controller start
```

## Stop

```bash
./controller stop
```
