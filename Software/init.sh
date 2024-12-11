#!/bin/bash
# /etc/init.d/vue_hb

### BEGIN INIT INFO
# Provides:          vue_hb
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Vivid Unit Extender heartbeat daemon script
# Description:       This survice runs vue_hb process in the background and submits heartbeat and temperature data.
### END INIT INFO

case "$1" in
    start)
        echo "Running vue_hb..."
				export DISPLAY=":0.0"
        /usr/bin/vue_hb > /var/log/vue_hb.log 2>&1 &
				sleep 1
				vue_hb_pid=$(pidof vue_hb)
				echo $vue_hb_pid > /var/run/vue_hb.pid
        ;;
    stop)
        echo "Stopping vue_hb..."
				vue_hb_pid=$(cat /var/run/vue_hb.pid)
				kill -9 $vue_hb_pid
        ;;
		status)
				if pidof myservice > /dev/null; then
					echo "vue_hb is running"
        else
					echo "vue_hb is not running"
        fi
				;;
    *)
        echo "Usage: /etc/init.d/vue_hb start|stop|status"
        exit 1
        ;;
esac

exit 0
