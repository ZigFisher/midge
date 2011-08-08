
. /www/oem.sh

L1(){
	[ -n "$2" ] && echo "<strong><a href='?controller=$2'>$1</a></strong><br>" \
			|| echo "<strong>$1</strong><br>"
}

L2(){
	local class=navlnk
	[ -n "$2" ] && echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>" \
		|| echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span class='$class'>$1</span><br>"
}

L3(){
	local class=navlnk
	[ -n "$3" ] && class="$3"
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='?controller=$2' class='$class'>$1</a><br>"
}

L1 System
	L2 Info 'welcome'
	L2 General 	'general'
	L2 Security 'passwd'
		L3 'PPP secrets' 'auth_ppp'
	L2 Time		'time'
	L2 Logging	'logging'

	# SHDSL section
	slots=`kdb get sys_pcitbl_slots`
	have_ifs=0
	
	for s in $slots; do
		type=`kdb get sys_pcitbl_s${s}_iftype`
		if [ "$type" != "$MR16H_DRVNAME" ] && [ "$type" != "$MR17H_DRVNAME" ]; then
			continue
		fi
		
		# output title
		if [ "$have_ifs" -eq 0 ]; then
			L2 SHDSL
			have_ifs=1
		fi
		unset num
		num=0
		for i in `kdb get sys_pcitbl_s${s}_ifaces`; do
			class=""
			[ "$FORM_iface" = "$i" ] && class="navlnk_a"
			L3	$i "dsl&iface=$i&pcislot=$s&pcidev=$num" $class
			num=`expr $num + 1`
		done
	done

	# EOC section
	if [ -f "/sbin/eoc-info" ]; then
		eval `/sbin/eoc-info -sr`
		
		if [ -n "$eoc_channels" ]; then
	    	L2 SHDSL-EOC
			L3 Profiles 'dsl-eoc&profiles=1'
		    for i in $eoc_channels; do
				unset ifname ifrole
				class=""
				ifname=${i%.*}
				[ "$FORM_iface" = "$ifname" ] && class="navlnk_a"
				L3	$ifname "dsl-eoc&iface=$ifname" $class
	    	done
		fi
	fi

	# E1 section
	slots=`kdb get sys_pcitbl_slots`
	have_ifs=0
	for s in $slots; do
		type=`kdb get sys_pcitbl_s${s}_iftype`
		if [ "$type" != "$MR16G_DRVNAME" ] && [ "$type" != "$MR17G_DRVNAME" ]; then
			continue
		fi
		
		# output title
		if [ "$have_ifs" -eq 0 ]; then
			L2 E1
			have_ifs=1
		fi
		unset num
		num=0
		for i in `kdb get sys_pcitbl_s${s}_ifaces`; do
			class=""
			[ "$FORM_iface" = "$i" ] && class="navlnk_a"
			L3	$i "e1&iface=$i&pcislot=$s&pcidev=$num" $class
			num=`expr $num + 1`
		done
	done

	# RS232 section
	slots=`kdb get sys_pcitbl_slots`
	have_ifs=0
	for s in $slots; do
		type=`kdb get sys_pcitbl_s${s}_iftype`
		if [ "$type" != "$MR17S_DRVNAME" ]; then
			continue
		fi
		
		# output title
		if [ "$have_ifs" -eq 0 ]; then
			L2 RS232
			have_ifs=1
		fi
		unset num
		num=0
		for i in `kdb get sys_pcitbl_s${s}_ifaces`; do
			class=""
			[ "$FORM_iface" = "$i" ] && class="navlnk_a"
			L3	$i "rs232&node=$i&pcislot=$s&pcidev=$num" $class
			num=`expr $num + 1`
		done
	done


	L2 Switch	'adm5120sw'
	L2 Multiplexing	'multiplexing'
	
	# check that VoIP module is installed
	[ "x$(kdb get sys_voip_present)" == "x1" ] && L2 VoIP 'voip'	
	
	L2 DNS		'dns'
	
L1 Network
	L2 Interfaces ifaces
	for i in `kdb get sys_ifaces`; do
		lclass=''
		lpage=''
		if [ "$controller" = "iface" ]; then
		    [ "$FORM_iface" = "$i" ] && lclass="navlnk_a"
			[ -n "$FORM_page" ] && lpage="&page=$FORM_page"
		fi
		L3	$i "iface&iface=${i}${lpage}" $lclass
	done
	L2 Firewall	fw
		L3 Filter	"fw&table=filter"
		L3 NAT		"fw&table=nat"
	L2 IPSec ipsec
	
L1 Services
	L2 "DNS Server" dns_server
	L2 "DHCP Server" dhcp_server
	L2 "PPTP Server" pptp_server
L1 Tools 
	L2 syslog	"tools&page=syslog"
	L2 dmesg	"tools&page=dmesg"
	L2 ping	"tools&page=ping"
	L2 mtr	"tools&page=mtr"
	L2 dig	"tools&page=dig"
	L2 tcpdump	"tools&page=tcpdump"
	L2 reboot	"tools&page=reboot"
L1 Configuration
	L2 backup	"cfg&page=backup"
	L2 restore	"cfg&page=restore"
L1 Expert
	L2 kdb	"expert&page=kdb"
	L2 "kdb set"	"expert&page=kdb_set"

#	<a href="javascript:showhide('diag','tri_diag')">
#					<img src="img/tri_c.gif" id="tri_diag" width="14" height="10" border="0"></a><strong><a href="javascript:showhide('diag','tri_diag')" class="navlnk">Diagnostics</a></strong><br>
#					<span id="diag"></span>
