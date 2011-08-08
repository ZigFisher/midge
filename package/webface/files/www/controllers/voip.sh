#!/usr/bin/haserl

	page=${FORM_page:-settings}
	subsys="voip"
	PORTSINFO="/proc/driver/sgatab/channels"
	PORTS_INFO_FULL=`cat $PORTSINFO`

	case $page in
		'settings')
			kdb_vars="str:sys_voip_settings_codec_ext_quality str:sys_voip_settings_codec_int_quality str:sys_voip_settings_log int:sys_voip_settings_rtp_port_first int:sys_voip_settings_rtp_port_last"
			;;
		'sip')
			kdb_vars="str:sys_voip_sip_registrar str:sys_voip_sip_username str:sys_voip_sip_password str:sys_voip_sip_user_sip_uri int:sys_voip_sip_chan"
			;;
		'hotline')
			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				kdb_vars="$kdb_vars bool:sys_voip_hotline_${portnum}_hotline"
				kdb_vars="$kdb_vars str:sys_voip_hotline_${portnum}_number"
				kdb_vars="$kdb_vars str:sys_voip_hotline_${portnum}_comment"
			done
			;;
		'sound')
			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_oob"
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_oob_play"
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_neventpt"
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_neventplaypt"
				kdb_vars="$kdb_vars int:sys_voip_sound_${portnum}_cod_tx_vol"
				kdb_vars="$kdb_vars int:sys_voip_sound_${portnum}_cod_rx_vol"
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_vad"
				kdb_vars="$kdb_vars str:sys_voip_sound_${portnum}_hpf"
			done
			;;
	esac
	
	render_save_stuff

	eval `kdb -qq ls sys_voip_*`
	
	render_page_selection "" settings "Settings" sip "SIP settings" route "Route table" address "Address book" hotline "Hotline" sound "Sound settings" 
	
	render_form_header
	render_input_field hidden page page "$page"

	case $page in 
		'settings')
			help_1="voip.settings"
			help_2=""
			render_table_title "VoIP Settings"
			
			# sys_voip_settings_rtp_port_first
			tip=""
			desc="Begin of ports range to use for RTP"
			validator="$tmtreq $validator_ipport"
			render_input_field text "RTP port start" sys_voip_settings_rtp_port_first						
			
			# sys_voip_settings_rtp_port_last
			tip=""
			desc="End of ports range to use for RTP"
			validator="$tmtreq $validator_ipport"
			render_input_field text "RTP port end" sys_voip_settings_rtp_port_last						
			
			# sys_voip_settings_codec_ext_quality
			tip="Quality of calls through SIP-server"
			desc="External call quality"
			render_input_field select "External quality" sys_voip_settings_codec_ext_quality speed "Speed" quality "Quality"
			
			# sys_voip_settings_codec_int_quality
			tip="Quality of calls between routers"
			desc="Internal call quality"
			render_input_field select "Internal quality" sys_voip_settings_codec_int_quality speed "Speed" quality "Quality"

			# sys_voip_settings_log
			tip=""
			desc="Level of logging"
			render_input_field select "Logging level" sys_voip_settings_log 0 "0" 1 "1" 2 "2" 3 "3" 4 "4" 5 "5" 6 "6" 7 "7" 8 "8" 9 "9"

			render_submit_field
			;;
		'sip')
			help_1="voip.sip"
			help_2=""
			render_table_title "SIP settings"
			
			# sys_voip_sip_registrar
			tip="f.e. <b>sip:server</b>"
			desc="SIP registrar to register on"
			validator="$tmtreq $validator_voip_registrar"
			render_input_field text "Registrar" sys_voip_sip_registrar
			
			# sys_voip_sip_username
			tip="f.e <b>user</b>"
			desc="Username on SIP registrar"
			validator="$tmtreq tmt:message='Please enter username'"
			render_input_field text "Username" sys_voip_sip_username
			
			# sys_voip_sip_password
			tip=""
			desc="Password on SIP registrar"
			validator="$tmtreq tmt:message='Please enter password'"
			render_input_field password "Password" sys_voip_sip_password
			
			# sys_voip_sip_user_sip_uri
			tip="f.e. <b>sip:user@server</b>"
			desc="User SIP URI"
			validator="$tmtreq $validator_voip_sip_uri"
			render_input_field text "User SIP URI" sys_voip_sip_user_sip_uri
			
			# sys_voip_sip_chan
			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				porttype=`echo $port | awk -F ':' '{print $2}'`
				[ $porttype == "FXS" ] && fxs="$fxs $portnum $portnum"
			done
			tip=""
			desc="FXS channel for incoming SIP-calls"
			validator=""
			render_input_field select "FXS channel" sys_voip_sip_chan $fxs
			
			render_submit_field
			;;
		'route')
			help_1="voip.route"
			help_2=""
			render_table_title "Route table"
			render_iframe_list "voip_route_table"
			;;
		'address')
			help_1="voip.address"
			help_2=""
			render_table_title "Address book"
			render_iframe_list "voip_address_book"
			;;
		'hotline')
			help_1="voip.hotline"
			help_2=""
			render_table_title "Hotline settings"
			echo "
				<tr><td colspan=\"2\">
				<table width=\"600px\" border=\"1\" style=\"border: solid 1px; 1px; 1px; 1px;\">
					<tr align='center'>
						<td>Channel</td><td>Type</td><td>Hotline</td>
						<td>Complete number</td><td>Comment</td>
					</tr>"

			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				porttype=`echo $port | awk -F ':' '{print $2}'`
				
				echo "<tr><td>$portnum</td><td>$porttype</td>"

				# sys_voip_hotline_${portnum}_hotline
				tip="Enable hotline for this channel"
				validator=""
				id="sys_voip_hotline_${portnum}_hotline"
				render_input_td_field checkbox sys_voip_hotline_${portnum}_hotline

				# sys_voip_hotline_${portnum}_number
				tip=""
				validator="tmt:required='conditional' tmt:dependonbox='sys_voip_hotline_${portnum}_hotline' $validator_voip_complete_number"
				render_input_td_field $disabled text sys_voip_hotline_${portnum}_number
				
				# sys_voip_hotline_${portnum}_comment
				tip=""
				validator="$validator_comment"
				render_input_td_field $disabled text sys_voip_hotline_${portnum}_comment
				
				echo "</tr>"
			done
			
			echo "</table></td></tr>"
			render_submit_field
			;;
		'sound')
			help_1="voip"
			help_2=""
			render_table_title "Sound settings"
			echo "
				<tr><td colspan=\"2\">
				<table width=\"600px\" border=\"1\" style=\"border: solid 1px; 1px; 1px; 1px;\">
					<tr align='center'>
						<td>Channel</td><td>OOB</td><td>OOB_play</td><td>nEventPT</td>
						<td>nEventPlayPT</td><td>COD_Tx_vol</td><td>COD_Rx_vol</td>
						<td>VAD</td><td>HPF</td>
					</tr>"

			for port in $PORTS_INFO_FULL; do
				portnum=`echo $port | awk -F ':' '{print $1}'`
				
				echo "<tr><td>$portnum</td>"	
				
				# sys_voip_sound_${portnum}_oob
				default="default"
				render_input_td_field select sys_voip_sound_${portnum}_oob default "default" in-band "in-band" out-of-band "out-of-band" both "both" block "block"			
				
				# sys_voip_sound_${portnum}_oob_play
				default="default"
				render_input_td_field select sys_voip_sound_${portnum}_oob_play default "default" play "play" mute "mute" play_diff_pt "play_diff_pt"
				
				# sys_voip_sound_${portnum}_neventpt
				default="0x62"
				render_input_td_field text sys_voip_sound_${portnum}_neventpt
				
				# sys_voip_sound_${portnum}_neventplaypt
				default="0x62"
				render_input_td_field text sys_voip_sound_${portnum}_neventplaypt

				# sys_voip_sound_${portnum}_cod_tx_vol
				default="0"
				render_input_td_field select sys_voip_sound_${portnum}_cod_tx_vol $(for i in `seq -24 2 24`; do echo $i $i; done)
				
				# sys_voip_sound_${portnum}_cod_rx_vol
				default="0"
				render_input_td_field select sys_voip_sound_${portnum}_cod_rx_vol $(for i in `seq -24 2 24`; do echo $i $i; done)

				# sys_voip_sound_${portnum}_vad
				default="off"
				render_input_td_field select sys_voip_sound_${portnum}_vad on "on" off "off" g711 "g711" CNG_only "CNG_only" SC_only "SC_only"
				
				# sys_voip_sound_${portnum}_hpf
				default="0"
				render_input_td_field select sys_voip_sound_${portnum}_hpf 0 "off" 1 "on"
				
				echo "</tr>"
			done
			
			echo "</table></td></tr>"
			render_submit_field
			;;
	esac
	
	render_form_tail
