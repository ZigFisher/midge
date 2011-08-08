#!/usr/bin/haserl
	
	item=$FORM_item
	iface=$FORM_iface
	subsys="network.$iface"
	
	# calculate next classid identifier
	if [ "$REQUEST_METHOD" = POST -a -n "$FORM_additem" ]; then
		max=0
		for key in `kdb kls sys_iface_${iface}_qos_htb_class_*`; do
			val=`kdb get $key`
			eval "$val"
			cur=`echo $classid |cut -d ':' -f2`
			if [ $cur -gt $max ]; then
				max=$cur
			fi
		done
		next=$(($max+1))
		FORM_classid="1:$next"
	fi

	# generate list of available classes
	CLASSES="1:0 root"
	for key in `kdb kls sys_iface_${iface}_qos_htb_class_*`; do
		val=`kdb get $key`
		eval "$val"
		CLASSES="$CLASSES $classid $name"
	done
	unset name enabled parent classid rate ceil qdisc _qdisc

	# replace spaces in qdisc with '#' for saving
	_qdisc=`echo $FORM_qdisc |sed s/" "/#/g`
	
	eval_string="export FORM_$item=\"name=$FORM_name enabled=$FORM_enabled parent=$FORM_parent classid=$FORM_classid rate=$FORM_rate ceil=$FORM_ceil qdisc=$_qdisc\""
	render_popup_save_stuff
	
	# replace '#' with spacing for edit displaying
	qdisc=`echo $qdisc |sed s/#/" "/g`

	render_form_header qos_class_edit
	help_1="htb"
	help_2="htb_class_add"
	render_table_title "QoS class edit" 2
	render_popup_form_stuff
	
	render_input_field hidden iface iface "$iface"
	render_input_field hidden classid classid "$classid"
	
	# enabled
	desc="Check this item to enable rule"
	validator='tmt:required="true"'
	render_input_field checkbox "Enable" enabled	
	
	# name
	desc="Name of class"
	validator="$tmtreq $validator_rulename"
	render_input_field text "Short name" name
		
	# parent
	desc="Name of parent class"
	render_input_field select "Parent class" parent $CLASSES
	
	# rate
	desc="Class rate"
	tip="Unit can be: <br/><b>kbit</b>, <b>Mbit</b> - for bit per second<br/> and <b>kbps</b>, <b>Mbps</b> - for bytes per second"
	validator="$tmtreq $validator_rate"
	render_input_field text "Rate" rate

	# ceil
	desc="Maximum rate"
	tip="Unit can be: <br/><b>kbit</b>, <b>Mbit</b> - for bit per second<br/> and <b>kbps</b>, <b>Mbps</b> - for bytes per second"
	validator="$validator_rate"
	render_input_field text "Ceil" ceil
	
	# qdisc
	desc="Qdisc for this class"
	tip="Optional qdisc for this class. For example, you can enter <b>esfq limit 128 depth 128 divisor 10 hash classic perturb 15</b> or <b>sfq perturb 10</b>, etc."
	validator=""
	render_input_field text "Qdisc" qdisc
	
	render_submit_field
	render_form_tail

# vim:foldmethod=indent:foldlevel=1