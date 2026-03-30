#!/bin/bash
# run_bbs.sh — Launch OrbitBBS QEMU VM + tcpser
# Usage: ./run_bbs.sh [start|stop|status|setup]

PROJ=/home/nick/Projects/Code_Projects/orbitbbs
HDA="$PROJ/base-dos.qcow2"
HDB="$PROJ/feeds.img"
PIDFILE=/tmp/orbitbbs.pid
TCPSER_PID=/tmp/orbitbbs-tcpser.pid
LOGFILE=/tmp/orbitbbs-qemu.log
TCPSER="$PROJ/tcpser/tcpser"
TELNET_PORT=2323

cmd="${1:-start}"

case "$cmd" in

  start|setup)
    if [ -f "$PIDFILE" ] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
      echo "OrbitBBS is already running (PID $(cat "$PIDFILE"))."
      exit 1
    fi

    if [ ! -f "$HDA" ]; then
      echo "ERROR: base disk not found: $HDA"
      exit 1
    fi

    if [ "$cmd" = "setup" ]; then
      echo "Starting OrbitBBS VM in SETUP mode (GTK display — run with DISPLAY=:1)..."
      QEMU_ARGS=(
        -hda "$HDA"
        -m 16
        -serial null
        -display gtk
      )
      INSTALL_IMG="$PROJ/install.img"
      if [ -f "$INSTALL_IMG" ]; then
        QEMU_ARGS+=(-hdb "$INSTALL_IMG")
      fi
      echo "Close the SDL window when done."
      qemu-system-i386 "${QEMU_ARGS[@]}"
      exit 0
    fi

    # Normal (headless) start
    QEMU_ARGS=(
      -hda "$HDA"
      -m 16
      -serial pty
      -display none
      -pidfile "$PIDFILE"
      -daemonize
    )

    echo "Starting OrbitBBS QEMU VM..."
    qemu-system-i386 "${QEMU_ARGS[@]}" > "$LOGFILE" 2>&1
    if [ $? -ne 0 ]; then
      echo "ERROR: QEMU failed to start. Check $LOGFILE."
      exit 1
    fi

    sleep 1
    PID=$(cat "$PIDFILE" 2>/dev/null)
    PTY=$(grep "char device redirected" "$LOGFILE" | awk '{print $5}')

    if [ -z "$PTY" ]; then
      echo "ERROR: could not determine PTY. Check $LOGFILE."
      exit 1
    fi

    # Start tcpser in background
    "$TCPSER" -d "$PTY" -s 38400 -p "$TELNET_PORT" -l 1 -i "s0=1" \
      >> /tmp/orbitbbs-tcpser.log 2>&1 &
    echo $! > "$TCPSER_PID"

    echo "OrbitBBS started."
    echo "  QEMU PID  : $PID"
    echo "  tcpser PID: $(cat "$TCPSER_PID")"
    echo "  PTY       : $PTY"
    echo "  Telnet    : localhost:$TELNET_PORT"
    ;;

  stop)
    STOPPED=0

    # Stop tcpser first
    if [ -f "$TCPSER_PID" ]; then
      TPID=$(cat "$TCPSER_PID")
      if kill -0 "$TPID" 2>/dev/null; then
        echo "Stopping tcpser (PID $TPID)..."
        kill "$TPID"
      fi
      rm -f "$TCPSER_PID"
      STOPPED=1
    fi

    # Stop QEMU
    if [ -f "$PIDFILE" ]; then
      PID=$(cat "$PIDFILE")
      if kill -0 "$PID" 2>/dev/null; then
        echo "Stopping OrbitBBS VM (PID $PID)..."
        kill "$PID"
        STOPPED=1
      else
        echo "VM PID $PID not running. Cleaning up stale pidfile."
      fi
      rm -f "$PIDFILE"
    else
      echo "No pidfile found — VM may not be running."
    fi

    [ $STOPPED -eq 1 ] && echo "Stopped."
    ;;

  status)
    if [ -f "$PIDFILE" ] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
      PID=$(cat "$PIDFILE")
      PTY=$(grep "char device redirected" "$LOGFILE" 2>/dev/null | awk '{print $5}')
      TPID=$(cat "$TCPSER_PID" 2>/dev/null)
      echo "OrbitBBS is RUNNING."
      echo "  QEMU PID  : $PID"
      echo "  tcpser PID: ${TPID:-not started}"
      echo "  PTY       : ${PTY:-check $LOGFILE}"
      echo "  Telnet    : localhost:$TELNET_PORT"
    else
      echo "OrbitBBS is STOPPED."
    fi
    ;;

  *)
    echo "Usage: $0 [start|stop|status|setup]"
    exit 1
    ;;

esac
