function generateMenu() {
	$("#menu").empty();
	addItem("System", "Info", "info");
	addItem("System", "Webface", "webface");
	addItem("System", "General", "general");
	addItem("System", "Security", "security");
	addItem("System", "DNS", "dns");
	addItem("System", "Time", "time");
	addItem("System", "Logging", "logging");
	addItem("System", "Tools", "tools");
	addItem("System", "Reboot", "reboot");
	addItem("System", "Configuration", "cfg");
	addItem("System", "Upload file", "upload");
	addItem("System", "Console", "console");
	addItem("Network", "Firewall", "fw");
	addItem("Network:Dynamic interfaces", "Manage", "dynamic_ifaces");
	addItem("Hardware", "Switch", "adm5120sw");
	addItem("Services", "DHCP server", "dhcp");
	addItem("Services", "DNS server", "dns_server");

	/* if we have support for linkdeps */
	if (config.getCachedOutput("linkdeps") == "1") {
		addItem("Hardware", "Linkdeps", "linkdeps");
	}

	/* if we have interfaces with multiplexing support, add item to the menu */
	if (config.getParsed("sys_mux_ifaces").length > 0) {
		addItem("Hardware", "Multiplexing", "multiplexing");
	}

	/* Add VoIP controller */
	if (config.get("sys_voip_present") == "1") {
		/* get VoIP channels list */
		config.runCmd("/bin/cat /proc/driver/sgatab/channels", "voipChannels");
		addItem("Hardware:VoIP", "Settings", "voipSettings");
        addItem("Hardware:VoIP", "Hotline", "voipHotline");
        addItem("Hardware:VoIP", "VF", "voipVF");
        addItem("Hardware:VoIP", "Routes", "voipRoutes");
        addItem("Hardware:VoIP", "Phone book", "voipPhoneBook");
        addItem("Hardware:VoIP", "Audio", "voipAudio");
        addItem("Hardware:VoIP", "Codecs", "voipCodecs");
        addItem("Hardware:VoIP", "Jitter buffer", "voipJitterBuffer");
        addItem("Hardware:VoIP", "Echo", "voipEcho");
        addItem("Hardware:VoIP", "Dial mode", "voipDialMode");
	}

	/* get array of PCI slots */
	var slots = config.getParsed("sys_pcitbl_slots");

	/* generate list of SHDSL/E1/RS232 interfaces */
	$.each(slots, function(num, pcislot) {
		var type = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
		var ifaces = config.getParsed($.sprintf("sys_pcitbl_s%s_ifaces", pcislot));
		var rs232Defined = false;

		/* get or create array with info for module of current type */
		var ifaceInfo = config.getData(type);
		if (!ifaceInfo) {
			ifaceInfo = [];
			config.saveData(type, ifaceInfo);
		}
		
		/* go through ifaces of this slot */
		$.each(ifaces, function(num, iface) {
			/* add info about this interface */
			ifaceInfo.push({"iface": iface, "pcislot": pcislot, "pcidev": num});

			switch (type) {
				/* SHDSL */
				case config.getOEM("MR16H_DRVNAME"):
				case config.getOEM("MR17H_DRVNAME"):
					if (type == config.getOEM("MR17H_DRVNAME")) {
						var confPath = $.sprintf("%s/%s/sg_private", config.getOEM("sg17_cfg_path"), iface);

						config.runCmd($.sprintf("/bin/cat %s/chipver", confPath));
						config.runCmd($.sprintf("/bin/cat %s/pwr_source", confPath));
					}

					addItem("Hardware:SHDSL", iface, "dsl", [iface, pcislot, num]);
					break;

				/* E1 */
				case config.getOEM("MR16G_DRVNAME"):
				case config.getOEM("MR17G_DRVNAME"):
					config.runCmd($.sprintf("[ -f /sys/class/net/%s/hw_private/muxonly ] && cat /sys/class/net/%s/hw_private/muxonly || echo -n 0",
							iface, iface), $.sprintf("muxonly_%s", iface));
					addItem("Hardware:E1", iface, "e1", [iface, pcislot, num]);
					break;

				/* RS232 */
				case config.getOEM("MR17S_DRVNAME"):
					if (!rs232Defined) {
						rs232Defined = true;
						config.runCmd($.sprintf("[ -f /sys/bus/pci/drivers/%s/%s/dev_type ] && cat /sys/bus/pci/drivers/%s/%s/dev_type || echo -n 'undefined'",
								config.getOEM("MR17S_DRVNAME"), iface, config.getOEM("MR17S_DRVNAME"),
								iface), $.sprintf("rs232Type_%s", pcislot));
					}
					addItem("Hardware:RS232", iface, "rs232", [iface, pcislot, num]);
					break;
			}
		});
	});

	/* generate list of network interfaces */
	var ifaces = config.getParsed("sys_ifaces");
	$(ifaces).each(function(name, iface) {
		/* add dynamic interfaces */
		if (iface.search(/\w+\d+v\d+/) != -1 || iface.search(/eth|dsl|E1/) == -1) {
			addItem("Network:Dynamic interfaces", iface, "iface", [iface]);
		/* add physical interfaces */
		} else {
			addItem("Network:Static interfaces", iface, "iface", [iface]);
		}
	});

	/* generate menu */
	$("#menu").treeview({
		"unique": true,
		"collapsed": true,
		"persist": "cookie"
	});
}

/* WF2 logs */
var wf2Logs = new function() {
	var logsNum = 40;
	var logsMax = 45;
	this.logs = [];

	this.addLog = function(title, text) {
		this.logs.push({"title": title, "text": text});
		
		/* rotate log */
		if (this.logs.length > logsMax) {
			this.logs = this.logs.slice(this.logs.length - logsNum);
		}
	};
};

/* device config */
var config = new Config();

$(document).ready(function() {
	/* Set AJAX options. Without this option IE caches all AJAX requests */
	$.ajaxSetup({
		cache: false
	});

	/* load KDB settings */
	config.loadKDB();
	config.loadOEM();

	/* load firmware version */
	config.runCmd("/bin/cat /etc/version");

	/* get availability for context-help */
	config.runCmd("[ -r '/www/help/index.html' ] && echo -n 1 || echo -n 0", "context-help");

	/* get availability for linkdeps */
	config.runCmd("[ -r '/etc/linkdeps' ] && echo -n 1 || echo -n 0", "linkdeps");

	/* set page title to router's hostname */
	document.title = config.get("sys_hostname");

	/* set interface lguage */
	var lang = config.get("sys_interface_language");
	if (lang) {
		$.gt.load(lang, $.sprintf("translation/%s.json", lang));
	}

	/* on click on status bar with CTRL key, show debug */
	$("#status").click(function(e) {
		if (e.ctrlKey == true) {
            Controllers.debug();
		}
	});
	
	/* add status bar content */
	$("#status").html(
		$.sprintf("%s: <b>%s</b>, %s: <b><span id='status_state'>%s</span></b>, %s: <b><span id='status_tasks'>%s</span></b>, %s: <b><span id='status_ajax'>%s</span></b>",
			_("Hostname"), config.get("sys_hostname"), _("status"), _("online"), _("tasks"), _("none"),
			_("ajax"), _("none"))
	);
	
	/* add status bar tip */
	$("#status").attr("title",
		_("|<ul><li>Hostname - device's hostname;</li><li>Status - is device online or offline;</li><li>Tasks - number of performing and queuened tasks;</li><li>Ajax - number of active ajax requests.</li></ul><br>You can click here with CTRL pressed to open debug panel.")
		).tooltip({"track": true, "showBody": "|"});

	/* call info controller when all config.runCmd will be finished */
	config.onCmdCacheFinish(function() {
		generateMenu();
		
		config.onCmdCacheFinish(function() {
			Controllers.info();

			/* check router every 10 seconds */
			config.startCheckStatus(10);
		});
	});
});
