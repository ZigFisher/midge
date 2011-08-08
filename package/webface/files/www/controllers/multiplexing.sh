#!/usr/bin/haserl
	require_js_file "mux.js"

	MXCONFIG=`which mxconfig`

	ifaces=`mxconfig -l |awk '{print $1}' | sed 's/://g'`

	if [ $REQUEST_METHOD = POST ]; then
		unset kdb_vars _clkr

		for i in $ifaces; do
			if [ "x${i%%_[0-9]*}x" = "xE1x" ]; then
				rate="mxsmap"
			else
				rate="mxrate"
			fi
			
			# if the CLKR was disabled in the form, we should manually set it to 0
			eval '_clkr=$'FORM_sys_mux_${i}_clkr
			[ -z "$_clkr" ] && _clkr="0"
			export FORM_sys_mux_${i}_clkr=$_clkr
			
			kdb_vars="$kdb_vars bool:sys_mux_${i}_mxen"
			kdb_vars="$kdb_vars int:sys_mux_${i}_clkm"
			kdb_vars="$kdb_vars int:sys_mux_${i}_clkab"
			kdb_vars="$kdb_vars int:sys_mux_${i}_clkr"
			kdb_vars="$kdb_vars int:sys_mux_${i}_rline"
			kdb_vars="$kdb_vars int:sys_mux_${i}_tline"
			kdb_vars="$kdb_vars int:sys_mux_${i}_rfs"
			kdb_vars="$kdb_vars int:sys_mux_${i}_tfs"
			kdb_vars="$kdb_vars str:sys_mux_${i}_${rate}"
		done
			
		subsys="mux"
		save "$subsys" "$kdb_vars" 
		render_save_message
	fi
	
	eval `$kdb -qq list sys_mux_*`
	
	render_form_header mux
	render_table_title "Multiplexing" 2
	
	echo "
		<tr><td colspan=\"2\">
		<table width=\"800px\" border=\"1\" style=\"border: solid 1px 1px; 1px; 1px;\">
			<tr align='center'>
				<td>DEV</td><td>MXEN</td><td>CLKM</td><td>CLKAB</td><td>CLKR</td><td>RLINE</td>
				<td>TLINE</td><td>RFS</td><td>TFS</td><td>MXRATE/MXSMAP</td>
			</tr>"

	for i in $ifaces; do
		echo "<tr><td>$i</td>"
		
		# sys_mux_${i}_mxen
		tip="Enable multiplexing on this interface"
		render_input_td_field checkbox sys_mux_${i}_mxen
		
		# sys_mux_${i}_clkm
		id="clkm_$i"
		onchange="OnChangeMuxCLKM(this);"
		tip="Select interface mode: <i>clock-master</i> or <i>clock-slave</i>"
		render_input_td_field select sys_mux_${i}_clkm 0 "clock-slave" 1 "clock-master"
		
		# sys_mux_${i}_clkab
		tip="Select interface clock domain: <i>A</i> or <i>B</i>"
		render_input_td_field select sys_mux_${i}_clkab 0 "A" 1 "B"
		
		# sys_mux_${i}_clkr
		# if CLKM is slave or not set — disable CLKR
		unset _clkm disabled
		eval '_clkm=$'sys_mux_${i}_clkm
		[ -z "$_clkm" -o "$_clkm" -eq "0" ] && disabled="-d" 
		id="clkr_$i"
		tip="Select clock source: <i>remote</i> or <i>local</i> (for <b>clock-master</b> interface only)"
		render_input_td_field $disabled select sys_mux_${i}_clkr 0 "local" 1 "remote"
		
		# sys_mux_${i}_rline
		tip="Enter rline number (<i>0-15</i>)"
		validator="$tmtreq $validator_muxline"
		default="0"
		render_input_td_field text sys_mux_${i}_rline
		
		# sys_mux_${i}_tline
		tip="Enter tline number (<i>0-15</i>)"
		validator="$tmtreq $validator_muxline"
		default="0"
		render_input_td_field text sys_mux_${i}_tline
		
		# sys_mux_${i}_rfs
		tip="Enter recieve frame start number (<i>0-255</i>)"
		validator="$tmtreq $validator_muxfs"
		default="0"
		render_input_td_field text sys_mux_${i}_rfs
		
		# sys_mux_${i}_tfs
		tip="Enter transmit frame start number (<i>0-255</i>)"
		validator="$tmtreq $validator_muxfs"
		default="0"
		render_input_td_field text sys_mux_${i}_tfs
		
		# sys_mux_${i}_(mxrate/mxsmap)
		if [ "x${i%%_[0-9]*}x" = "xE1x" ]; then
			rate="mxsmap"
			unset mxsmap
			eval `mxconfig --iface $i --list |grep mxsmap |sed "s/$i://g"`
			kdb set "sys_mux_${i}_mxsmap=$mxsmap"
			eval `$kdb -qq list sys_mux_${i}_mxsmap`
		else
			rate="mxrate"
		fi
		tip="Enter <b>mxrate</b> for DSL interface or <b>mxsmap</b> for E1 interface (f.e. <i>12</i> or <i>0-31</i>)"
		validator="$tmtreq $validator_muxrate"
		default="0"
		render_input_td_field text sys_mux_${i}_${rate}
		echo "</tr>"
	done
	
	echo "</table></td></tr>"
	
	render_submit_field
	render_form_tail
	
	echo "	<tr><td colspan='2'>
			Definition of parameters:
			<ul>
				<li>MXEN - enable multiplexing</li>
				<li>CLKM - clock-master</li>
				<li>CLKAB - clock domain</li>
				<li>CLKR - clock source</li>
				<li>RLINE - Transmit multiplexer bus line</li>
				<li>TLINE - Receive multiplexer bus line</li>
				<li>RFS - Receive Frame Start</li>
				<li>TFS - Transmit Frame Start</li>
				<li>MXRATE/MXSMAP - multiplexing rate (use for SHDSL) / Slotmap used for multiplexing (use for E1)</li>
			</ul>
		</td></tr>
		"
	
	echo "<tr><td colspan='2'><b>Checking status:</b></td></tr><tr><td colspan='2'>"
	check=`$MXCONFIG --check`
	if [ -n "$check" ]; then
		echo "Errors detected:</td></tr><tr><td><pre>$check</pre>"
	else
		echo "No errors found"
	fi
	echo "</td></tr>"
