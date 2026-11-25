#!/usr/bin/env bash
# Throwaway databases for the live tests.
#
#   bash scripts/livedb.sh up [pg] [redis]     start fresh instances (all by default)
#   bash scripts/livedb.sh down [pg] [redis]   stop them and delete their data
#   bash scripts/livedb.sh status
#   bash scripts/livedb.sh run <command...>    up, run the command, down
#
# The defaults match CMakeUserPresets.json, so tests need no extra setup.

set -euo pipefail

ROOT=${FLUXMT_LIVEDB_DIR:-/tmp/fluxmt-livedb}

PG_BIN=${PG_BIN:-$(ls -d /usr/lib/postgresql/*/bin 2>/dev/null | sort -V | tail -n1 || true)}
PG_PORT=${FLUXMT_TEST_PG_PORT:-5432}
PG_DB=${FLUXMT_TEST_PG_DB:-fluxmt_test}
PG_USER=${FLUXMT_TEST_PG_USER:-fluxmt_test}

REDIS_PORT=${FLUXMT_TEST_REDIS_PORT:-6379}

ALL="pg redis"


die() {
    echo "livedb: $*" >&2
    exit 1
}

listening() {
    (exec 3<>"/dev/tcp/127.0.0.1/$1") 2>/dev/null
}

wait_port() {
    for _ in $(seq 1 100); do
        listening "$1" && return 0
        sleep 0.1
    done

    return 1
}


# pg ----------------------------------------------------------------------------

pg_running() {
    [[ -n $PG_BIN && -f $ROOT/pg/postmaster.pid ]] && "$PG_BIN/pg_ctl" -D "$ROOT/pg" status >/dev/null 2>&1
}

pg_up() {
    [[ -n $PG_BIN ]] || die "postgresql binaries not found (set PG_BIN)"
    pg_running && { echo "pg: already up on $PG_PORT"; return; }
    listening "$PG_PORT" && die "port $PG_PORT is taken: sudo systemctl disable --now postgresql"

    rm -rf "$ROOT/pg"
    mkdir -p "$ROOT"

    "$PG_BIN/initdb" -D "$ROOT/pg" -U "$PG_USER" --auth=trust --no-sync -E UTF8 --locale=C.UTF-8 >/dev/null
    "$PG_BIN/pg_ctl" -D "$ROOT/pg" -l "$ROOT/pg.log" -w start >/dev/null \
        -o "-p $PG_PORT -k $ROOT -c listen_addresses=127.0.0.1 -c fsync=off -c synchronous_commit=off -c full_page_writes=off"
    "$PG_BIN/createdb" -h 127.0.0.1 -p "$PG_PORT" -U "$PG_USER" "$PG_DB"

    echo "pg: up on $PG_PORT, database $PG_DB, user $PG_USER"
}

pg_down() {
    pg_running && "$PG_BIN/pg_ctl" -D "$ROOT/pg" stop -m immediate -s
    rm -rf "$ROOT/pg" "$ROOT/pg.log" "$ROOT"/.s.PGSQL.*
    echo "pg: down"
}

pg_status() {
    if pg_running; then echo "pg: up on $PG_PORT"; else echo "pg: down"; fi
}


# redis -------------------------------------------------------------------------

redis_running() {
    [[ -f $ROOT/redis.pid ]] && kill -0 "$(cat "$ROOT/redis.pid")" 2>/dev/null
}

redis_up() {
    redis_running && { echo "redis: already up on $REDIS_PORT"; return; }
    listening "$REDIS_PORT" && die "port $REDIS_PORT is taken: sudo systemctl disable --now redis-server"

    rm -rf "$ROOT/redis"
    mkdir -p "$ROOT/redis"

    redis-server --port "$REDIS_PORT" --bind 127.0.0.1 --save "" --appendonly no \
        --dir "$ROOT/redis" --pidfile "$ROOT/redis.pid" --logfile "$ROOT/redis.log" --daemonize yes >/dev/null
    wait_port "$REDIS_PORT" || die "redis did not start, see $ROOT/redis.log"

    echo "redis: up on $REDIS_PORT"
}

redis_down() {
    redis_running && kill "$(cat "$ROOT/redis.pid")"
    rm -rf "$ROOT/redis" "$ROOT/redis.pid" "$ROOT/redis.log"
    echo "redis: down"
}

redis_status() {
    if redis_running; then echo "redis: up on $REDIS_PORT"; else echo "redis: down"; fi
}


# main --------------------------------------------------------------------------

each() {
    local action=$1
    shift

    for name in ${*:-$ALL}; do
        declare -F "${name}_${action}" >/dev/null || die "unknown target: $name"
        "${name}_${action}"
    done
}

case ${1:-} in
    up)     shift; each up "$@" ;;
    down)   shift; each down "$@"; rmdir "$ROOT" 2>/dev/null || true ;;
    status) each status ;;
    run)
        shift
        [[ $# -gt 0 ]] || die "run needs a command"
        trap 'each down >/dev/null; rmdir "$ROOT" 2>/dev/null || true' EXIT
        each up
        "$@"
        ;;
    *) sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
esac
