/* Global hash for controllers */
var Controllers = {
	/* container for displaying controllers content */
	container: "#container",
	
	/* delegates to Page() defined in widgets.js */
	Page: function() {
		return new Page(this.container);
	}
};

Controllers.info = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "info",
		"name": "System information",
		"func": function() {
			var c, field;
			c = page.addContainer("info");
			c.addTitle("System information");
			
			field = {
				"type": "html",
				"name": "sys_hostname",
				"text": "Hostname",
				"kdb": "sys_hostname"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "version",
				"text": "Firmware version",
				"str": config.getCachedOutput("/bin/cat /etc/version")
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "time",
				"text": "Time",
				"cmd": "/bin/date"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "uptime",
				"text": "Uptime",
				"cmd": "/usr/bin/uptime |/usr/bin/cut -f1 -d ','"
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "la",
				"text": "Load average",
				"cmd": "/bin/cat /proc/loadavg",
				"tip": "The first three numbers represent the number of active tasks on the system - processes that are actually running - averaged over the last 1, 5, and 15 minutes. The next entry shows the instantaneous current number of runnable tasks - processes that are currently scheduled to run rather than being blocked in a system call - and the total number of processes on the system. The final entry is the process ID of the process that most recently ran."
			};
			c.addWidget(field);
			
			/* Hardware section */
			page.addBr("info");
			c = page.addContainer("info");
			c.addTitle("Hardware information");
			
			field = {
				"type": "html",
				"name": "ethernet",
				"text": "Ethernet",
				"str": function() {
					var ethIfaces = "";
					$(config.getParsed("sys_ifaces")).each(function(name, iface) {
						if (iface.search(/eth/) != -1) {
							ethIfaces += iface + "<br/>";
						}
					});
					return ethIfaces ? ethIfaces : "none";
				}()
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "shdsl",
				"text": "SHDSL",
				"str": function() {
					var info = "";
					
					var ifaces = config.getData(config.getOEM("MR16H_DRVNAME"));
					if (ifaces) {
						$.each(ifaces, function(num, ifaceInfo) {
							info += $.sprintf("%s (%s)<br/>", ifaceInfo.iface,
									mr16hModuleName(ifaceInfo.pcislot));
						});
					}
					
					ifaces = config.getData(config.getOEM("MR17H_DRVNAME"));
					if (ifaces) {
						$.each(ifaces, function(num, ifaceInfo) {
							info += $.sprintf("%s (%s)<br/>", ifaceInfo.iface,
									mr17hModuleName(ifaceInfo.iface, ifaceInfo.pcislot));
						});
					}
					
					return info ? info : "none";
				}()
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "e1",
				"text": "E1",
				"str": function() {
					var info = "";

					var ifaces = config.getData(config.getOEM("MR16G_DRVNAME"));
					if (ifaces) {
						$.each(ifaces, function(num, ifaceInfo) {
							info += $.sprintf("%s (%s)<br/>", ifaceInfo.iface,
									mr16gModuleName(ifaceInfo.pcislot));
						});
					}
					
					ifaces = config.getData(config.getOEM("MR17G_DRVNAME"));
					if (ifaces) {
						$.each(ifaces, function(num, ifaceInfo) {
							info += $.sprintf("%s (%s)<br/>", ifaceInfo.iface,
									mr17gModuleName(ifaceInfo.pcislot));
						});
					}
					
					return info ? info : "none";
				}()
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "rs232",
				"text": "RS232",
				"str": function() {
					var info = "";

					var ifaces = config.getData(config.getOEM("MR17S_DRVNAME"));
					if (ifaces) {
						$.each(ifaces, function(num, ifaceInfo) {
							info += $.sprintf("%s (%s)<br/>", ifaceInfo.iface,
									mr17sModuleName(ifaceInfo.pcislot));
						});
					}
					
					return info ? info : "none";
				}()
			};
			c.addWidget(field);
			
			field = {
				"type": "html",
				"name": "voip",
				"text": "VoIP",
				"str": function() {
					var info = {};
					
					var channels = config.getCachedOutput("voipChannels");
					
					if (channels) {
						$.each(channels.split("\n"), function(num, channel) {
							if (channel.length == 0) {
								return true;
							}

							var channel = channel.split(":");
							var channelIdx = parseInt(channel[0], 10);
							if (!channelIdx && channelIdx != 0) {
								return true;
							}
							if (channelIdx <= 7) {
								channelIdx = 1;
							} else if (channelIdx <= 15) {
								channelIdx = 2;
							} else if (channelIdx <= 23) {
								channelIdx = 3;
							} else if (channelIdx <= 31) {
								channelIdx = 4;
							}

							if (!info[channelIdx]) {
								info[channelIdx] = $.sprintf("Module %s: ", channelIdx);
							}

							info[channelIdx] += $.sprintf("<br/>&nbsp;&nbsp;&nbsp;&nbsp;%s (%s)",
									channel[0], channel[1]);
						});

						var result = "";
						$.each(info, function(num, val) {
							if (result.length > 0) {
								result += "<br/>";
							}
							result += val;
						});
						return result;
					} else {
						return "none";
					}
				}()
			};
			c.addWidget(field);
		}
	});
	
	page.generateTabs();
};

Controllers.webface = function() {
	var page = this.Page();

	page.addTab({
		"id": "webface",
		"name": "Webface",
		"func": function() {
			var c, field;
			c = page.addContainer("webface");
			c.addTitle("Webface settings");

			field = {
				"type": "select",
				"name": "sys_interface_language",
				"text": "Interface language",
				"options": {"en": "English", "ru": _("Russian")} 
			};
			c.addWidget(field);
		
			c.addSubmit({"reload": true});
		}
	});
	
	page.generateTabs();
};

Controllers.general = function() {
	var page = this.Page();
	page.setHelpPage("begin");

	page.addTab({
		"id": "general",
		"name": "General",
		"func": function() {
			var c, field;
			c = page.addContainer("general");
			c.setHelpSection("hostname");
			c.addTitle("General settings");

			field = {
				"type": "text",
				"name": "sys_hostname",
				"text": "Hostname",
				"descr": "Device's hostname.",
				"validator": {"required": true, "alphanumU": true},
				"message": "Enter correct hostname"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};

Controllers.security = function() {
	var page = this.Page();
	page.setHelpPage("begin");
	
	page.addTab({
		"id": "security",
		"name": "Security",
		"func": function() {
			var c, field;
			c = page.addContainer("security");
			c.setSubsystem("security");
			c.setHelpSection("passwd");
			c.addTitle("Webface password");

			field = { 
				"type": "password",
				"name": "htpasswd",
				"id": "htpasswd",
				"text": "Password",
				"descr": "Password for webface user <i>admin</i>.",
				"validator": {"required": true, "alphanumU": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "htpasswd2",
				"id": "htpasswd2",
				"text": "Repeat password",
				"descr": "Password for webface user <i>admin</i>.",
				"validator": {"equalTo": "#htpasswd"}
			};
			c.addWidget(field);
		
			c.addSubmit({
				/* set a value to a subsystem field before a form will be submitted */
				"preSubmit": function() {
					$("#subsystem").val($.sprintf("security.htpasswd.%s", $("#htpasswd").val()));
				},
				"onSubmit": function() {
					$("#htpasswd").val("");
					$("#htpasswd2").val("");
				}
			});
			
			/* system (console) password */
			page.addBr("security");
			c = page.addContainer("security");
			c.setHelpSection("passwd");
			c.addTitle("System console password");
		
			/*
			 * We set subsystem via API (with id 'subsystem') in previous form,
			 * so, for excluding duplication of the id, we set it manually here.
			 */
			field = { 
				"type": "hidden",
				"name": "subsystem",
				"id": "subsystem2"
			};
			c.addWidget(field);

			field = { 
				"type": "password",
				"name": "passwd",
				"id": "passwd",
				"text": "Password",
				"descr": "Password for system user <i>root</i> to log in via console.",
				"validator": {"required": true, "alphanumU": true}
			};
			c.addWidget(field);
			
			field = { 
				"type": "password",
				"name": "passwd2",
				"id": "passwd2",
				"text": "Repeat password",
				"descr": "Password for system user <i>root</i> to log in via console.",
				"validator": {"equalTo": "#passwd"}
			};
			c.addWidget(field);
		
			c.addSubmit({
				/* set a value to a subsystem field before a form will be submitted */
				"preSubmit": function() {
					$("#subsystem2").val($.sprintf("security.passwd.%s", $("#passwd").val()));
				},
				"onSubmit": function() {
					$("#passwd").val("");
					$("#passwd2").val("");
				}
			});
		}
	});
	
	page.generateTabs();
};

Controllers.dns = function() {
	var page = this.Page();
	page.setSubsystem("dns");
	
	page.addTab({
		"id": "dns",
		"name": "DNS",
		"func": function() {
			var c, field;
			c = page.addContainer("dns");
			c.addTitle("DNS settings");

			field = {
				"type": "text",
				"name": "sys_dns_nameserver",
				"text": "DNS server 1",
				"descr": "IP address of upstream dns server.",
				"tip": "DNS server used for resolving domain names. E.g., 192.168.2.1.",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);
		
			field = {
				"type": "text",
				"name": "sys_dns_nameserver2",
				"text": "DNS server 2",
				"descr": "IP address of upstream dns server.",
				"tip": "DNS server used for resolving domain names. E.g., 192.168.2.1.",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);

			field = { 
				"type": "text",
				"name": "sys_dns_domain",
				"text": "Domain",
				"descr": "Your domain.",
				"tip": "Domain for this router. E.g., localnet."
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};

Controllers.time = function() {
	var page = this.Page();
	page.setSubsystem("time");
	
	page.addTab({
		"id": "time",
		"name": "Time settings",
		"func": function() {
			var c, field;
			c = page.addContainer("time");
			c.addTitle("Time settings");

			field = {
				"type": "html",
				"name": "time",
				"text": "Time",
				"descr": "Current date and time on the device.",
				"cmd": "/bin/date"
			};
			c.addWidget(field);

			field = { 
				"type": "checkbox",
				"name": "sys_ntpclient_enabled",
				"text": "Use time synchronizing",
				"descr": "Check this item if you want use time synchronizing.",
				"tip": "Time synchronization via NTP protocol."
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_ntpclient_server",
				"text": "Time server",
				"descr": "Hostname or IP address of time server."
			};
			c.addWidget(field);
			
			field = {
				"type": "select",
				"name": "sys_timezone",
				"text": "Time zone",
				"descr": "Time zone.",
				"options": function() {
					var tz = {};
					for (var i = -12; i <= 12; i++) {
						var offset = i > 0 ? "+" + i : "" + i;
						
						/*
						 * "Confusingly, the offset in the TZ value specifies the time value
						 * you must add to the local time to get a UTC value, so this is positive
						 * if the local time zone is west of the Prime Meridian and negative if 
						 * it is east.", http://martybugs.net/wireless/openwrt/timesync.cgi
						 * So, we inverse the value of GMT offset.
						 */
						var gmtOffset = i * -1 > 0 ? "+" + i * -1 : "" + i * -1;
						tz[offset] = "GMT" + (gmtOffset != "0" ? gmtOffset : "");
					}
					return tz;
				}()
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_time_auto_switch",
				"text": "Auto winter/summer time",
				"descr": "Auto switch to winter/summer time.",
				"tip": "Adds one hour to local time in summer."
			};
			c.addWidget(field);
			
			/* update date and time after form saving */
			c.addSubmit({
				"onSuccess": function() {
					config.cmdExecute({
						"cmd": "/bin/date",
						"container": "#td_time > span.htmlWidget"
					});
				}
			});
		}
	});
	
	page.generateTabs();
};

Controllers.logging = function() {
	var page = this.Page();
	page.setHelpPage("logging");
	page.setSubsystem("logging");
	
	page.addTab({
		"id": "logging",
		"name": "Logging",
		"func": function() {
			var c, field;
			c = page.addContainer("logging");
			c.addTitle("Logging settings");

			field = { 
				"type": "select",
				"name": "sys_log_dmesg_level",
				"text": "Kernel console priority logging",
				"descr": "Set the level at which logging of messages is done to the console",
				"options": {"1": "1", "2": "2", "3": "3", "4": "4", "5": "5", "6": "6", "7": "7"}
			};
			c.addWidget(field);
		
			field = { 
				"type": "select",
				"name": "sys_log_buf_size",
				"text": "Circular buffer",
				"descr": "Circular buffer size",
				"options": {"0": "0k", "8": "8k", "16": "16k", "32": "32k", "64": "64k", "128": "128k",
							"256": "256k", "512": "512k"}
			};
			c.addWidget(field);
			
			field = { 
				"type": "checkbox",
				"name": "sys_log_remote_enabled",
				"text": "Enable remote syslog logging",
				"descr": "Check this item if you want to enable remote logging"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "sys_log_remote_server",
				"text": "Remote syslog server",
				"descr": "Domain name or ip address of remote syslog server"
			};
			c.addWidget(field);
		
			c.addSubmit();
		}
	});
	
	page.generateTabs();
};

Controllers.tools = function() {
	var page = this.Page();

	page.addTab({
		"id": "syslog",
		"name": "syslog",
		"func": function() {
			var c;
			c = page.addContainer("syslog");
			c.addTitle("syslog");
			
			/* working directory for script is ./wf2/sh, where execute.cgi is located */
			c.addConsole("/sbin/logread");
		}
	});
	
	page.addTab({
		"id": "dmesg",
		"name": "dmesg",
		"func": function() {
			var c;
			c = page.addContainer("dmesg");
			c.addTitle("dmesg");
			c.addConsole("/bin/dmesg");
		}
	});
	
	page.addTab({
		"id": "ping",
		"name": "ping",
		"func": function() {
			var c, field;
			c = page.addContainer("ping");
			c.addTitle("ping");
			
			field = { 
				"type": "text",
				"name": "host",
				"text": "Host",
				"defaultValue": "localhost"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "count",
				"text": "Count",
				"defaultValue": "5"
			};
			c.addWidget(field);
			
			c.addRun("/sbin/ping -c %ARG %ARG", "count", "host");
		}
	});
	
	page.addTab({
		"id": "mtr",
		"name": "mtr",
		"func": function() {
			var c, field;
			c = page.addContainer("mtr");
			c.addTitle("mtr");
			
			field = { 
				"type": "text",
				"name": "mtr_host",
				"text": "Host",
				"defaultValue": "localhost"
			};
			c.addWidget(field);
			
			field = { 
				"type": "text",
				"name": "mtr_count",
				"text": "Count",
				"defaultValue": "5"
			};
			c.addWidget(field);
			
			c.addRun("/usr/sbin/mtr -r -n -s 100 -c %ARG %ARG", "mtr_count", "mtr_host");
		}
	});
	
	page.generateTabs();
};

Controllers.reboot = function() {
	var page = this.Page();
	
	page.addTab({
		"id": "reboot",
		"name": "Reboot",
		"func": function() {
			var c;
			c = page.addContainer("reboot");
			c.addTitle("Reboot");
			
			c.addAction("Reboot", "/sbin/reboot");
		}
	});
	
	page.generateTabs();
};

Controllers.cfg = function() {
	var page = this.Page();
	page.setHelpPage("cfg");
	
	page.addTab({
		"id": "backup",
		"name": "Backup configuration",
		"func": function() {
			var c, field;
			c = page.addContainer("backup");
			c.setHelpSection("backup");
			c.addTitle("Backup configuration");
			
			/* tell what to do cfg.cgi */
			field = {
				"type": "hidden",
				"name": "act",
				"defaultValue": "backup"
			};
			c.addWidget(field);
			
			c.addSubmitNoAjax({
				"submitName": "Backup",
				"formAction": "/cfg.cgi"
			});
		}
	});
	
	page.addTab({
		"id": "restore",
		"name": "Restore configuration",
		"func": function() {
			var c, field;
			c = page.addContainer("restore");
			c.setHelpSection("restore");
			c.addTitle("Restore configuration");
			
			/* tell what to do cfg.cgi */
			field = {
				"type": "hidden",
				"name": "act",
				"defaultValue": "restore"
			};
			c.addWidget(field);
			
			field = {
				"type": "file",
				"name": "uploadfile",
				"text": "Backuped configuration",
				"descr": "Restore configuration from file."
			};
			c.addWidget(field);
			
			c.addSubmitNoAjax({
				"submitName": "Restore",
				"formAction": "/cfg.cgi",
				"method": "post",
				"encType": "multipart/form-data"
			});
		}
	});
	
	page.addTab({
		"id": "default",
		"name": "Default configuration",
		"func": function() {
			var c, field;
			c = page.addContainer("default");
			c.setHelpSection("default");
			c.addTitle("Restore default configuration");
			
			/* tell what to do cfg.cgi */
			field = {
				"type": "hidden",
				"name": "act",
				"defaultValue": "default"
			};
			c.addWidget(field);
			
			c.addSubmitNoAjax({
				"submitName": "Restore",
				"formAction": "/cfg.cgi"
			});
		}
	});
	
	page.generateTabs();
};

Controllers.debug = function() {
	var page = this.Page();

	page.addTab({
		"id": "debug",
		"name": "Debug",
		"func": function() {
			var c, field;
			c = page.addContainer("debug");
			c.addTitle("Debug info");

			$.each(wf2Logs.logs, function(num, log) {
				field = {
					"type": "html",
					"name": "debug_" + num,
					"text": log.title,
					"str": log.text
				};
				c.addWidget(field);
			});
		}
	});
	
	page.generateTabs();
};

Controllers.upload = function() {
	var page = this.Page();

	page.addTab({
		"id": "upload",
		"name": "Upload file",
		"func": function() {
			var c, field;
			c = page.addContainer("upload");
			c.addTitle("Upload file to device");

			field = {
				"type": "text",
				"name": "path",
				"text": "Path",
				"descr": "Where to place uploaded file.",
				"defaultValue": "/root",
				"validator": {"required": true}
			};
			c.addWidget(field);

			field = {
				"type": "file",
				"name": "uploadfile",
				"text": "File",
				"descr": "File to upload.",
				"validator": {"required": true}
			};
			c.addWidget(field);

			c.addSubmitNoAjax({
				"submitName": "Upload",
				"formAction": "sh/upload.cgi",
				"method": "post",
				"encType": "multipart/form-data"
			});
		}
	});

	page.generateTabs();
};

Controllers.console = function() {
	var page = this.Page();

	page.addTab({
		"id": "console",
		"name": "Console",
		"func": function() {
			var cmd = "";
			var p = page.getRaw("console");

			var consoleDiv = $.create("div", {"id": "consoleDiv", "className": "pre scrollable",
					"tabindex": "0"}, "# ").appendTo(p).focus();

			/* this element is used as anchor for scrolling */
			$.create("span", {"id": "bottomAnchor"}, "&nbsp;").appendTo(p);

			/* add blinking cursor */
			var cursor = $.create("span", {"id": "consoleCursor"}, "_").appendTo(consoleDiv);
			var cursorAnimate = function() {
				var nextDisplay = cursor.css("display") == "inline" ? "none" : "inline";
				cursor.css("display", nextDisplay);
			};
			setInterval(cursorAnimate, 500);

			/* span for current command text */
			var cmdSpan = $.create("span").insertBefore(cursor);

			/* keypress event handler */
			var keypressDisabled = false;
			var onKeypress = function(src) {
				var ch;

				if (keypressDisabled) {
					return false;
				}

				/* ENTER pressed, send text to router */
				if (src.keyCode == 13) {
					config.cmdExecute({
						"cmd": cmd,
						"formatData": true,
						"callback": function(data) {
							/* enable keypress event */
							keypressDisabled = false;

							$("#executingCmd").remove();

							cursor.before(data);
							cursor.before("# ");

							/* add new span for command text */
							cmdSpan = $.create("span").insertBefore(cursor);
							
							consoleDiv.scrollTo($("#bottomAnchor"), 700);
						}
					});

					/* disable keypress event */
					keypressDisabled = true;

					/* clear cmd */
					cmd = "";

					cursor.before("<br/>");
					cursor.before($.create("span", {"id": "executingCmd"}, _("executing command...")));
					consoleDiv.scrollTo($("#bottomAnchor"), 700);

					/* disable statndart event handler */
					return false;
				} else if (src.keyCode == 16 || src.keyCode == 17 || src.keyCode == 18
						|| src.keyCode == 37 || src.keyCode == 38 || src.keyCode == 39
						|| src.keyCode == 40 || src.keyCode == 8) {
					return false;
				}

				/* get pressed character depending on browser */
				if (src.which == null) {
					/* IE */
					ch = String.fromCharCode(src.keyCode);
				} else if (src.which > 0) {
					/* others */
					ch = String.fromCharCode(src.which);
				}

				/* append pressed character to command */
				cmdSpan.append(ch);
				cmd += ch;

				/* disable statndart event handler */
				return false;
			};

			/* when BACKSPACE pressed, remove last entered character */
			var onBackspace = function() {
				cmd = cmd.substring(0, cmd.length - 1);
				cmdSpan.text(cmd);

				return false;
			};

			consoleDiv.keypress(onKeypress);
			consoleDiv.keydown(function(src) {
				if (src.keyCode == 8) {
					return onBackspace();
				}
			});
		}
	});

	page.generateTabs();
};
