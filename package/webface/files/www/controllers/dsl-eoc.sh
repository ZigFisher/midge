#!/usr/bin/haserl

# SHDSL modules web-control through EOC daemon 
# Written by Polyakov A.U. <artpol84@gmail.com>

info=/sbin/eoc-info
config=/sbin/eoc-config

require_js_file "prototype.js"
require_js_file "dsl.js"

_eoc_settings()
{
	iface=$1


	#--------------- Apply changes ------------------#
	unset exec_error
	opts=""
	if [ -n "$FORM_mode" ] && [ "$FORM_mode" != "$type" ]; then
		case "$FORM_mode" in
		master)
			opts=$opts" -m1"
			type="master"
			;;
		slave)
			opts=$opts" -m0"
			type="slave"
			;;
		esac
	fi
	
	if [ -n "$FORM_adm_reg_num" ] && [ "$FORM_adm_reg_num" != "$adm_reg_num" ]; then
		opts=$opts" --reg-num=$FORM_adm_reg_num"
	fi
	if [ -n "$FORM_cprof" ] && [ "$FORM_cprof" != "$cprof" ]; then
		opts=$opts" --cprof=$FORM_cprof"
	fi

	if [ -n "$opts" ]; then
		# echo "$config -o channel -c$iface $opts -s"
		eval `$config -o channel -c$iface $opts -s`
		if [ "$eoc_error" -eq "1" ]; then
			ERROR_MESSAGE="$err_string"
			render_save_message_nohide
			exec_error=1
			unset eoc_error
		fi
		changes=1
	fi
	
	# ----- PBO-related changes
	opts=""
	if [ "$type" = "master" ]; then
		eval `$info -i$iface -ro`
		if [ "$eoc_error" -eq "1" ]; then
			ERROR_MESSAGE="$err_string"
			render_save_message_nohide
			exec_error=1
			unset eoc_error
		fi
	
		if [ "$FORM_hsubmit" = "1" ] && [ "$master" != "0" ]; then
			new_pbo_mode=0
			if [ "$FORM_pbo_mode" = "on" ]; then
				new_pbo_mode=1
			fi
			if [ "$new_pbo_mode" != "$pbo_mode" ]; then
				opts=$opts" -x$new_pbo_mode"
			fi
			if [ "$new_pbo_mode" = "1" ] &&
				[ "$FORM_pboval" != "$pbo_val" ]; then
				opts=$opts" -w$FORM_pboval"
			fi
			if [ -n "$opts" ]; then
				# echo "$config -ochannel -c$iface $opts <br>"
				$config -ochannel -c$iface $opts -s
				if [ "$eoc_error" -eq "1" ]; then
					ERROR_MESSAGE="$err_string"
					render_save_message_nohide
					exec_error=1
					unset eoc_error
				fi
				changes=1
			fi
		fi	
	fi
	
	if [ -n "$changes" ]; then
		if [ -z "$exec_error" ]; then		
			$config -us
			render_save_message
			render_js_refresh_window 300
			return
		else
			sleep 2
		fi
	fi	
	
	unset eoc_error
	eval `$info -tr`
	if [ "$eoc_error" -eq "1" ]; then
		echo "Error interface names"
		return
	fi


	render_form_header
	help_1="dsl-eoc.settings"
	help_2=""
	render_table_title "$iface settings" 2

	# sys_dsl_${iface}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" page settings
	render_input_field "hidden" "hidden" hsubmit "1"

  	# Interface mode
	mode=$type
	tip=""
	id='mode'
	desc="Interface mode"
	onchange="eocIfSettings();"
	render_input_field select "Mode" mode master 'Master' slave 'Slave'

	if [ "$type" = "master" ]; then
  		# Regenerators admin number
	    tip=""
   		desc="Number of installed regenerators (theoretical)"
	    render_input_field text "Regenerators" adm_reg_num
	fi
	
  	# Span Configuration profile
	unset tmp
	tmp=""
	for i in $cprof_list; do
		tmp=$tmp" $i $i"
	done
	tip=""
	desc="Span Configuration profile associated with channel $iface"
 	render_input_field select "ConfProfile" cprof $tmp



  	# Power Backoff
	# get info
	unset eoc_error
	eval `$info -i$iface -or`
	if [ "$eoc_error" != "1" ]; then
		id='hpboval'
		render_input_field "hidden" "hidden" hpboval "$pbo_val"
		
		tip=""
		id='pbomode'
		td_id='pbomode_td'
		desc='Example: 21:13:15, STU-C-SRU1=21,SRU1-SRU2=13,...'
		onchange="eocIfSettings();"
		render_input_field checkbox "PBO Forced" pbo_mode 
	fi

	run_js_code "eocIfSettings();"

	render_submit_field
	render_form_tail
}

_eoc_stat_general()
{
	iface=$1
	help_1="eoc"
	help_2=""
	render_table_title "$iface status" 2

	if [ "$link" -eq "0" ]; then
		link=offline
		real_regs=`expr $unit_num - 1`
	else
		link=online
		real_regs=`expr $unit_num - 2`
	fi
	[ "$real_regs" -lt 0 ] && real_regs=0
	
	tip=""
	desc="STU-C connected to STU-R"
	render_input_field static "Channel link" status "$link"

	tip=""
	desc="Regenerators in channel (real number)"
	render_input_field static "Regenerators" regs "$real_regs"

	tip=""
	desc="Number of wire pairs in channel"
	render_input_field static "Wire pairs" pairs "$loop_num"

	tip=""
	desc="Channel rate value"
	render_input_field static "Rate" regs "$rate"

	tip=""
	desc="Channel annex value"
	render_input_field static "Annex" regs "$annex"

	tip=""
	desc="Channel tcpam value"
	render_input_field static "Encoding" regs "$tcpam"
	
}

_eoc_stat_1dint()
{
	iface=$1
	u=$2
	s=$3
	l=$4
	
	help_1="eoc"
        help_2=""
	render_table_title "$iface $s Pair"`expr $l + 1`" 1 Day error intervals" 2
	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
		<tr align='center'>
			<td>Date</td><td>ES</td><td>SES</td><td>CV</td><td>LOSWS</td><td>UAS</td>
			<td>Monitoring(%)</td>
		</tr>"
	k=1
	while [ "$k" -le 30 ]; do
		unset unit side loop es ses crc losws uas eoc_error int_day int mon_pers
		eval `$info -r -i${iface} -u${u} -e${s} -l${l} -d$k`
		if [ "$eoc_error" -eq "1" ];  then
			break
		fi

		echo "<tr align='center'>"
		echo "<td>$int_day</td>"
					
		# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
		tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
		render_input_td_field static es "$es"
	
		# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
		tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
		render_input_td_field static ses "$ses"

		# Count of CRC anomalies on this endpoint since unit was last restarted
		tip="Count of CRC anomalies on this endpoint since unit was last restarted"
		render_input_td_field static crc "$crc"

		# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
		tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
		render_input_td_field static losws "$losws"

		# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
		tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
		render_input_td_field static uas "$uas"

		# Monitoring seconds
		tip="Monitored persentage"
		render_input_td_field static mon_pers "$mon_pers"
		echo "</tr>"
		k=`expr $int + 1`
	done
	echo "</table></td></tr>"
}


_eoc_stat_15mint()
{
	iface=$1
	u=$2
	s=$3
	l=$4
	
	help_1="eoc"
        help_2=""
	render_table_title "$iface $s Pair"`expr $l + 1`" 15 Minutes error intervals" 2
	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
		<tr align='center'>
			<td>Date</td><td>Start time</td><td>End time</td><td>ES</td><td>SES</td><td>CV</td>
			<td>LOSWS</td><td>UAS</td><td>Monitoring(%)</td>

		</tr>"
	k=1
	while [ "$k" -le 96 ]; do
		unset unit side loop es ses crc losws uas eoc_error int_day time_end time_start int
		eval `$info -r -i${iface} -u${u} -e${s} -l${l} -m$k`
		if [ "$eoc_error" -eq "1" ];  then
			break
		fi

		echo "<tr align='center'>"
		echo "<td>$int_day</td>"
		echo "<td>$time_start</td>"
		echo "<td>$time_end</td>"
					
		# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
		tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
		render_input_td_field static es "$es"
	
		# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
		tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
		render_input_td_field static ses "$ses"

		# Count of CRC anomalies on this endpoint since unit was last restarted
		tip="Count of CRC anomalies on this endpoint since unit was last restarted"
		render_input_td_field static crc "$crc"

		# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
		tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
		render_input_td_field static losws "$losws"

		# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
		tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
		render_input_td_field static uas "$uas"

		# Monitoring seconds
		tip="Monitored persentage"
		render_input_td_field static mon_pers "$mon_pers"
		echo "</tr>"


		k=`expr $int + 1`
	done
	echo "</table></td></tr>"
}

_eoc_stat_unit()
{
	iface=$1
	u=$2

	if [ -n "$FORM_relative" ]; then
		$info -r -i${iface} -u${u} -e${FORM_side} -l${FORM_loop} -v
	fi

	case "$u" in
	stu-c)
		sides="CustSide"
		sensors=0
		;;
	stu-r)
		sides="NetSide"
		sensors=0
		;;
	sru*)
		sensors=1
		sides="NetSide CustSide"
		;;
	esac

	help_1="eoc"
	help_2=""
	render_table_title "$iface state" 2

	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
			<tr align='center'>
				<td>Side</td><td>Pair</td><td>SNR</td><td>LoopAttn</td><td>ES</td><td>SES</td>
				<td>CV</td><td>LOSWS</td><td>UAS</td>
			</tr>"

	for s in $sides; do
		l=0
		while [ "$l" -lt "$loop_num" ]; do
			echo "<tr align='center'>"
			echo "<td>$s</td>"
			echo "<td>Pair"`expr $l + 1`"</td>"
		
			unset unit side loop snr lattn es ses crc losws uas
			eval `$info -r -i${iface} -u${u} -e${s} -l${l}`
			if [ "$eoc_error" = 1 ]; then
				echo "<td>Request error</td>"
			fi

			# SNR margin
			tip="SNR margin value"
			render_input_td_field static snr "$snr"
		
			# Loop Attenuation value
			tip="Loop Attenuation value"
			render_input_td_field static lattn "$lattn"
		
			# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
			tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
			render_input_td_field static es "$es"

			# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
			tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
			render_input_td_field static ses "$ses"

			# Count of CRC anomalies on this endpoint since unit was last restarted
			tip="Count of CRC anomalies on this endpoint since unit was last restarted"
			render_input_td_field static crc "$crc"

			# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
			tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
			render_input_td_field static losws "$losws"

			# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
			tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
			render_input_td_field static uas "$uas"
			echo "</tr>"
			l=`expr $l + 1`
		done
	done

	echo "</table></td></tr>"

	
	if [ "$sensors" = 1 ]; then

	    echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
			<tr align='center'>
				<td>Sensor #</td><td>Current state</td><td>Event Counter</td>
			</tr>"
	    eval `$info -r -i${iface} -u${u} --sensors`
	    for s in 1 2 3; do
		unset cstate cntr
		eval "cstate=\$sens${s}_cur"
		eval "cntr=\$sens${s}_cnt"
		echo "<tr align='center'>"
		echo "<td>$s</td>"
		echo "<td>$cstate</td>"
		echo "<td>$cntr</td>"
	    done
	    echo "</table></td></tr>"
	fi

	help_1="eoc"
        help_2=""
	render_table_title "$iface relative counters" 2

	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
			<tr align='center'>
				<td>Start date</td><td>Start time</td><td>Side</td><td>Pair</td>
				<td>ES</td><td>SES</td><td>CV</td><td>LOSWS</td><td>UAS</td>
				<td>Reset</td>
			</tr>"

	for s in $sides; do
		l=0
		while [ "$l" -lt "$loop_num" ]; do
		
			unset unit side loop tes tses tcrc tlosws tuas ttstamp
			eval `$info -r -i${iface} -u${u} -e${s} -l${l}`
			if [ "$eoc_error" = 1 ]; then
				echo "<tr><td colspan=\"11\">Request error</td></tr>"
				break
			fi
			
			render_form_header_light
			render_input_field "hidden" "hidden" iface $iface
			render_input_field "hidden" "hidden" relative "1"
			render_input_field "hidden" "hidden" page "statistic"
			render_input_field "hidden" "hidden" page_l2 "$u"
			render_input_field "hidden" "hidden" loop "$l"
			render_input_field "hidden" "hidden" side "$s"

			echo "<tr align='center'>"
			echo "<td>$tdate</td><td>$ttime</td><td>$s</td>"
			echo "<td>Pair"`expr $l + 1`"</td>"


			# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
			tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
			render_input_td_field static es "$tes"

			# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
			tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
			render_input_td_field static ses "$tses"

			# Count of CRC anomalies on this endpoint since unit was last restarted
			tip="Count of CRC anomalies on this endpoint since unit was last restarted"
			render_input_td_field static crc "$tcrc"

			# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
			tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
			render_input_td_field static losws "$tlosws"

			# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
			tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
			render_input_td_field static uas "$tuas"

			render_submit_field_light "Reset"
			render_form_tail_light

			echo "</tr>"
			l=`expr $l + 1`

		done
	done

	echo "</table></td></tr>"


	help_1="eoc"
        help_2=""
	render_table_title "$iface current intervals" 2

	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
			<tr align='center'>
				<td>Interval</td><td>Side</td><td>Pair</td>
				<td>ES</td><td>SES</td><td>CV</td><td>LOSWS</td><td>UAS</td>
				<td>Time elapsed</td>
			</tr>"

	for s in $sides; do
		l=0
		while [ "$l" -lt "$loop_num" ]; do
		
			unset unit side loop snr lattn m15es m15ses m15crc m15losws m15uas m15elaps d1es d1ses d1crc d1losws d1uas d1elaps
			eval `$info -r -i${iface} -u${u} -e${s} -l${l}`
			if [ "$eoc_error" = 1 ]; then
				echo "<td>Request error</td>"
			fi

			echo "<tr align='center'>"
			echo "<td>Curr 15 minutes</td>"
			echo "<td>$s</td>"
			echo "<td>Pair"`expr $l + 1`"</td>"

			# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
			tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
			render_input_td_field static es "$m15es"

			# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
			tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
			render_input_td_field static ses "$m15ses"

			# Count of CRC anomalies on this endpoint since unit was last restarted
			tip="Count of CRC anomalies on this endpoint since unit was last restarted"
			render_input_td_field static crc "$m15crc"

			# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
			tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
			render_input_td_field static losws "$m15losws"

			# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
			tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
			render_input_td_field static uas "$m15uas"

			# Time elapsed
			tip="15 minute interval time elapsed"
			render_input_td_field static elaps "$m15elaps"
			
			echo "</tr>"

			echo "<tr align='center'>"
			echo "<td>Curr 1 day</td>"
			echo "<td>$s</td>"
			echo "<td>Pair"`expr $l + 1`"</td>"

			# Count of Errored Seconds (ES) on this endpoint since unit was last restarted
			tip="Count of Errored Seconds (ES) on this endpoint since unit was last restarted"
			render_input_td_field static es "$d1es"

			# Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted
			tip="Count of Severely Errored Seconds (SES) on this endpoint since the xU was last restarted"
			render_input_td_field static ses "$d1ses"

			# Count of CRC anomalies on this endpoint since unit was last restarted
			tip="Count of CRC anomalies on this endpoint since unit was last restarted"
			render_input_td_field static crc "$d1crc"

			# Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted
			tip="Count of Loss of Sync Word (LOSW) Seconds since unit was last restarted"
			render_input_td_field static losws "$d1losws"

			# Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted
			tip="Count of Unavailable Seconds (UAS) on this endpoint since unit was last restarted"
			render_input_td_field static uas "$d1uas"

			# 1 day time elapsed
			tip="1 day time elapsed"
			render_input_td_field static uas "$d1elaps"

			echo "</tr>"
			
			l=`expr $l + 1`
		done
	done
	
	echo "</table></td></tr>"


	for s in $sides; do
		l=0
		while [ "$l" -lt "$loop_num" ]; do
			_eoc_stat_15mint $iface $u $s $l
			l=`expr $l + 1`
		done
	done

	for s in $sides; do
		l=0
		while [ "$l" -lt "$loop_num" ]; do
			_eoc_stat_1dint $iface $u $s $l
			l=`expr $l + 1`
		done
	done
}

_eoc_statistic()
{
	# Retreive data form eocd
#	unset error
#	eval `$info -ri${iface}`
#	if [ "$error" -eq "1" ]; then
#		echo "Error interface name"
#		return
#	fi

	tmp=""
	if [  "$unit_num" -ne "0" ]; then
		tmp="stu-c \"STU-C\" "
		if [ "$link" -eq "1" ]; then
			tmp=$tmp"stu-r \"STU-R\" "
			i=1
			while [ "$i" -le "$reg_num" ]; do
				tmp=$tmp"sru$i \"SRU$i\" " 
				i=`expr "$i" + 1`
			done
		else
			i=1
			while [ "$i" -lt "$unit_num" ]; do
				tmp=$tmp" sru$i \"SRU$i\"" 
				i=`expr "$i" + 1`
			done
		fi
	fi

	page_l2=${FORM_page_l2:-general} 
	eval "render_page_selection_l2 \"iface=$iface&page=$page\" general \"General\" $tmp"
	if [ "$page_l2" = "general" ]; then
		_eoc_stat_general $iface
	else
		_eoc_stat_unit $iface $page_l2
	fi
}

_eoc_profiles()
{
	# Save results comed from user
	if [ -n "$FORM_pname" ]; then
		unset rate annex power pname tcpam mrate
		# Rate selection type
		if [ "$FORM_rate" -eq "-1" ]; then
			rate="-l$FORM_mrate"
			kdb set sys_eocd_profile_${FORM_pname}=text
		else
			rate="-l$FORM_rate"
			kdb set sys_eocd_profile_${FORM_pname}=list
		fi

		[ -n "$FORM_annex" ] && annex="-n$FORM_annex"
		[ -n "$FORM_power" ] && power="-f$FORM_power"
		[ -n "$FORM_tcpam" ] && tcpam="-t${FORM_tcpam##tcpam}"

		unset eoc_error
		if [ -n "$FORM_nprofile" ]; then # New profile - add
			eval `$config -oconf-prof -a$FORM_pname $rate $annex $power $tcpam -s`
			#$config -oconf-prof -a$FORM_pname $rate $annex $power -s
		elif [ -n "$FORM_dprofile" ]; then # Delete profile
			eval `$config -oconf-prof -d$FORM_pname -s`
			#$config -oconf-prof -d$FORM_pname -s
		else # Change profile
			#echo "$config -oconf-prof -c$FORM_pname $rate $annex $power $tcpam -s<br>"
			eval `$config -oconf-prof -c$FORM_pname $rate $annex $power $tcpam -s`
		fi		
		if [ "$eoc_error" -eq "1" ]; then
			ERROR_MESSAGE="$err_string"
			render_save_message_nohide
		else
			$config -us
			render_save_message
		fi
	fi

	unset eoc_error
	eval `$info -ra`
	if [ "$eoc_error" -eq "1" ]; then
		return 1
	fi
	render_table_title "SHDSL configuration profiles" 2
	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\" id=\"cprof\">
			<tr align='center'>
				<td>Name</td><td>Rate</td><td>Annex</td><td>Power</td>
				<td>Encoding</td><td>Save Config</td><td>Del Config</tr>
			</tr>"
	k=1
	while [ "$k" -gt "0" ]; do
		unset pname rate annex power tcpam
		eval "pname=\${cprof$k}"
		eval "rate=\${rate$k}"
		eval "mrate=\${mrate$k}"
		eval "annex=\${annex$k}"
		eval "power=\${power$k}"
		eval "tcpam=\${tcpam$k}"
		if [ -z "$pname" ]; then
			break
		fi


		rtype=`kdb get sys_eocd_profile_$pname`
		mrate="$rate"
		if [ "$rtype" = "text" ]; then
			rate="-1"
		fi

		echo "<tr>"

		render_form_header_light "f$pname"
		render_input_field "hidden" "hidden" profiles "1"
		render_input_field "hidden" "hidden" pname "$pname"
		id="hmrate$k"
		render_input_field "hidden" "hidden" hmrate "$mrate"

		if [ "$pname" = "default" ]; then
			echo"<td>SET prefix to RO</td>"
			prefix="-d"	
			dis_set="disabled='true'"
		else 
			prefix=""
			dis_set=""
		fi


		# Profile name
		tip="Name"
		render_input_td_field static pname $pname
		
		# Profile rate
		echo -n "<td>"
		echo -n "<select $dis_set name='rate' class='edit' id='rate"$k"' onChange='eocProfiles();' tmt:errorclass='invalid'>"
		echo -n "<option value=$rate selected>$rate</option> </select>"
		echo -n "<input type='text' style='display:none' id='mrate"$k"' name='mrate' size='5' maxlength='5' value='"$mrate"'>"
		echo -n "</td>"
		#render_input_td_field $prefix select rate $rate $rate
		
		# Annex
		tip="Annex"
		render_input_td_field $prefix select annex AnnexA AnnexA AnnexB AnnexB

		# Power
		tip="Power"
		render_input_td_field $prefix select power on on off off

		# Encoding
		tip="Encoding"
		id="tcpam$k"
		onchange="eocProfiles();"	
		render_input_td_field $prefix select tcpam tcpam8 TCPAM8 tcpam16 TCPAM16 tcpam32 TCPAM32 tcpam64 TCPAM64 tcpam128 TCPAM128

		render_submit_field_light
		render_form_tail_light

		render_form_header_light "$pname"
		render_input_field "hidden" "hidden" profiles "1"
		render_input_field "hidden" "hidden" dprofile "1"
		render_input_field "hidden" "hidden" pname "$pname"
		render_submit_field_light "Delete"
		echo "</tr>"

		render_form_tail_light
		k=`expr $k + 1`
	done
	echo "</table></td></tr>"

	# Add new profile -------------------------------------------------------

	render_table_title "Add SHDSL configuration profiles" 2
	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\" id=\"cprof\">
			<tr align='center'>
				<td>Name</td><td>Rate</td><td>Annex</td><td>Power</td>
				<td>Encoding</td><td>Save Config</td>
			</tr>"

	echo "<tr>"

	render_form_header_light "$pname"
	render_input_field "hidden" "hidden" profiles "1"
	render_input_field "hidden" "hidden" nprofile "1"

	unset rate annex power pname
	# Profile name
	tip="Name"
	render_input_td_field text pname ""
	
	
	# Profile rate
	tip="Channel rate"
	id="rate"
	rate=192
	onchange="eocProfiles();"	
	render_input_td_field select rate 192 192
		
	# Annex
	tip="Annex"
	annex=AnnexA
	render_input_td_field select annex AnnexA AnnexA AnnexB AnnexB

	# Power
	tip="Power"
	power=off
	render_input_td_field select power on on off off

	# Encoding
	tip="Encoding"
	tcpam=tcpam16
	id="tcpam"
	onchange="eocProfiles();"	
	render_input_td_field select tcpam tcpam8 TCPAM8 tcpam16 TCPAM16 tcpam32 TCPAM32 tcpam64 TCPAM64 tcpam128 TCPAM128

	render_submit_field_light
	render_form_tail_light
	echo "</tr></table>"
	
	run_js_code "eocProfiles();"
}


if [ -n "$FORM_profiles" ]; then
	_eoc_profiles 
	exit 0
fi

iface="${FORM_iface}"

# Retreive general data form eocd
unset error
eval `$info -ri${iface}`
if [ "$error" -eq "1" ]; then
	echo "Error interface name"
	return
fi

if [ "$type" = "slave" ]; then
	_eoc_settings $iface
	exit 0
fi

page=${FORM_page:-settings} 
render_page_selection "iface=$iface" settings "Settings" statistic "Statistic"


case "$page" in
'settings')
	_eoc_settings $iface
	;;
'statistic')
	_eoc_statistic $iface
	;;
esac


