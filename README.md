# irods-docker-deploy
Deploy iRODS using Docker


## Build
Update configuration parameters set in `config.inc`.

```bash
./build
```

## Create Volumes
Creates Docker volumes set in `config.inc`.
By default, it creates three volumes, `irods_volume`, `db_volume`, and `db_backup_volume`.

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

## Backup DB Data
Backup DB data and creates a backup file on the volume `db_backup_volume`.

```bash
./controller backup_db
```

Use `cron` to backup DB data repeatedly. 


```bash
echo "0 1 * * * root bash ${PWD}/controller backup_db" | sudo tee -a /etc/crontab > /dev/null
```

