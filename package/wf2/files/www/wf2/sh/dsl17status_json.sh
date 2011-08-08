#!/bin/sh
# Returns SHDSL v17 status in JSON.
# First argument — interface name, second argument is optional, if set, it activates debug mode.

. /www/oem.sh

iface=$1
debug=$2
conf="$sg17_cfg_path/$iface/sg_private"
RESULT=""

# print indentation tabs, enabled only in debug mode
print_indent() {
	local level=$1
	
	if [ -n "$debug" ]; then
		local i=0
		while [ $i -lt $level ]; do
			RESULT="$RESULT \t"
			i=$((1+$i))
		done
	fi
}

# start JSON section
start_section() {
	local name=$1
	local level=$2
	
	print_indent $level
	
	RESULT="$RESULT \"$name\": {\n"
}

# end JSON section
end_section() {
	local level=$1
	local comma=$2
	
	print_indent $level
	
	[ -n "$comma" ] && RESULT="$RESULT },\n" || RESULT="$RESULT }\n"
}

# add variable to JSON
add_var() {
	local name=$1
	local value=$2
	local level=$3
	local comma=$4
	
	print_indent $level
	
	RESULT="$RESULT \"$name\": \"$value\""
	[ -n "$comma" ] && RESULT="$RESULT ,\n" || RESULT="$RESULT \n"
}

# begin JSON
RESULT="$RESULT {\n"

# PWR section
	start_section "pwr" 1
		pwr_presence=`/bin/cat $conf/pwr_source`
		
		if [ "$pwr_presence" = "1" ]; then
			add_var "presence" "$pwr_presence" 2 comma
			add_var "unb" "`/bin/cat $conf/pwrunb`" 2 comma
			add_var "ovl" "`/bin/cat $conf/pwrovl`" 2
		else
			add_var "presence" "$pwr_presence" 2
		fi
	end_section 1 comma
	
# LINK section
	start_section "link" 1
		link_state=`/bin/cat $conf/link_state`
		
		if [ "$link_state" = "1" ]; then
			add_var "link_state" "$link_state" 2 comma
			add_var "rate" "`/bin/cat $conf/rate`" 2 comma
			add_var "tcpam" "`/bin/cat $conf/tcpam`" 2 comma
			add_var "clkmode" "`/bin/cat $conf/clkmode`" 2 comma
			add_var "statistics_row" "`/bin/cat $conf/statistics_row`" 2
		else
			add_var "link_state" "$link_state" 2
		fi
	end_section 1 comma

# PBO section
	start_section "pbo" 1
		pbo_mode=`/bin/cat $conf/pbo_mode`
		
		if [ "$pbo_mode" = "Forced" ]; then
			add_var "mode" "$pbo_mode" 2 comma
			add_var "val" "`/bin/cat $conf/pbo_val`" 2
		else
			add_var "mode" "$pbo_mode" 2
		fi
	end_section 1

# finish JSON
RESULT="$RESULT }"

echo -e $RESULT
