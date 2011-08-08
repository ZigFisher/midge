#!/bin/sh
# (c) Vladislav Moskovets 2005
#

service_reload(){
	local service="$1"
	
	case "$service" in
	network*)
		# get interface name
		local iface=${service#*.}
		
		# if interface name is specified, restart only that interface
		if [ "$iface" != "network" ]; then
			/etc/init.d/network restart $iface
		# otherwise, restart full subsystem 
		else
			/etc/init.d/network restart
		fi
	;;
	dhcp*)
		[ -z "$iface" ] && iface=${service#*.}
		/etc/init.d/udhcpd restart $iface
	;;
	dns_server)
		/etc/init.d/bind restart
	;;
	# we can enable/disable multiplexing on module settings page, so restart
	# multiplexing on module settings change
	dsl*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/dsl restart $slot $dev
		/etc/init.d/mux start

		# restart EOCd
		/usr/bin/killall -HUP eocd
	;;
	eoc_profile)
		/usr/bin/killall -HUP eocd
	;;
	e1*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/e1 restart "$slot" "$dev"
		/etc/init.d/mux start
		/etc/init.d/network restart
	;;
	rs232*)
		tmp=${service#*.}
		slot=${tmp%.*}
		dev=${tmp#*.}
		/etc/init.d/rs232 restart "$slot" "$dev"
		/etc/init.d/mux start
	;;
	fw)
		/etc/init.d/fw restart
	;;
	logging)
		/etc/init.d/sysklog restart
	;;
	ipsec)
		/etc/templates/ipsec-tools.sh | /usr/sbin/setkey -c
	;;
	mux)
		/etc/init.d/mux start
	;;
	# VoIP in old web
	voip)
		/etc/init.d/rcvoip restart
	;;
	# VoIP in WF2
	svd*)
		/etc/init.d/rcvoip restart
	;;
	iface_del*)
		iface=${service#*.}
		/sbin/ifdown $iface 2>&1 | ${LOGGER}
	;;
	security*)
		tmp=${service#*.}
		form=${tmp%.*}
		passwd=${tmp#*.}

		case $form in
			htpasswd)
				echo $passwd | htpasswd /etc/htpasswd admin 2>&1 | $LOGGER
			;;
			passwd)
				(echo $passwd; sleep 1; echo $passwd) | passwd root 2>&1 | $LOGGER
			;;
		esac
	;;
	time)
		/etc/init.d/timesync start
	;;
	esac
}
