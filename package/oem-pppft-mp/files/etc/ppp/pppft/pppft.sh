#!/bin/sh

. /etc/ppp/pppft/pppft.cfg

LOGFILE="/etc/ppp/pppft/log"
PPPD_OPTIONS="modem lock noauth multilink passive"

LOG_WARN=0
LOG_ERR=1
LOG_NOTICE=0

log_err(){
	[ ! "$LOG_ERR" -eq 1 ] && return
	echo `date`": ERROR: "$1  >> $LOGFILE
}

log_warn(){
	[ ! "$LOG_WARN" -eq 1 ] && return
	echo `date`": WARNING: "$1  >> $LOGFILE
}

log_notice(){
	[ ! "$LOG_NOTICE" -eq 1 ] && return
	echo `date`": NOTICE: "$1  >> $LOGFILE
}


add_ttys()
{
	local tty_pid
	
	iface=$1
	shift
	
	ifindex=${iface#ppp}
	eval "locip=\${${iface}_locip}"
	eval "remip=\${${iface}_remip}"
	
	IP_PART=""
	if [ -n "$locip" ]; then
		IPPART="$locip"
		[ -n "$remip" ] && IPPART=$IPPART":$remip"
	fi
	for tty in $*; do
		if [ ! -c "/dev/$tty" ]; then
			log_err "No such device /dev/$tty. Skip"
			continue
		fi
		if [ -f "/var/lock/LCK..$tty" ]; then
			tty_pid=`cat "/var/lock/LCK..$tty" | awk '{print $(1)}'`
			if [ -d "/proc/$tty_pid" ]; then
				kill -KILL $tty_pid
			fi
		fi
		eval "TTYSPEED=\$${iface}_$tty"
		if [ -z "$TTYSPEED" ]; then
			TTYSPEED=115200
			log_error "$iface: No speed for $tty found, use default = 115200"
		fi
		log_notice "add_ttys: pppd $PPPD_OPTIONS unit $ifindex $IPPART $tty $TTYSPEED "
		pppd $tty $TTYSPEED $PPPD_OPTIONS $IPPART  
	done
}

check_setup_iface()
{
	local iface=$1
	local tmp
	local bkpdir
	local rttys
	local ttys
	local ppp_pid
	local tty_pid
	local ppp_master

	log_notice "Process IF=$iface"	
	#check if name
	tmp=`echo $iface | grep "^ppp[0-9][0-9]*$"`
	if [ -z "$tmp" ]; then
		log_err "Wrong ppp device name \"$iface\". Need pppN" 
		return
	fi
	
	# check ttys
	eval "ttys=\${${iface}_ttys}"
	if [ -z "$ttys" ]; then
		log_err "No ttys registered for $iface"
		return
	fi

	#check if interface is present
	ip link show $iface 1>/dev/null 2>/dev/null
	if [ "$?" -ne "0" ]; then
		log_notice "Device $iface is not present"
		add_ttys $iface $ttys
		return
	fi
	
	# check if master tty is connected
	ppp_pid=`cat /var/run/$iface.pid`
	ppp_master=0
	for i in /var/lock/LCK..*; do
		tty_pid=`cat $i`
		if [ "$tty_pid" -eq "$ppp_pid" ]; then
			ppp_master=1
		fi
	done
	if [ "$ppp_master" -eq 0 ]; then
		log_notice "Recreate $iface connection"
		kill -KILL "$ppp_pid"
		add_ttys $iface $ttys
		return
	fi

	# Check nonmaster tty's		
	for tty in $ttys; do
		log_notice "Process tty=$tty"
		# check if lock exist
		if [ -f "/var/lock/LCK..$tty" ]; then
			#check if corresponding process exist
			ppp_pid=`cat "/var/lock/LCK..$tty" | awk '{print $(1)}'`
			if [ -d "/proc/$ppp_pid" ]; then
				log_notice "tty=$tty has corresponding pppd with PID=$ppp_pid"
				continue
			fi
			log_notice "tty=$tty has NO corresponding pppd"
		fi
		log_notice "Assign $tty to $iface"
		add_ttys $iface $tty
	done
}

if [ "${ppp_mlink_en}" = 0 ]; then
	exit
fi

if [ -z "$ifaces" ]; then
	log_err "No interfaces to process. exit"
	exit 0
fi

for iface in $ifaces; do
	check_setup_iface $iface
done
