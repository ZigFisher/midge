/* Object for commands' queue */
function KDBQueue() {
	var queue = new Array();
	var block = false;
	var performingTask = false;
	
	this.getTasksNumber = function() {
		return performingTask ? queue.length + 1 : queue.length;
	};
	
	/*
	 * Update status.
	 */
	var updateMessage = function() {
		if (queue.length > 0) {
			$("#status_tasks").text(queue.length);
		} else {
			$("#status_tasks").text(_("none"));
		}
	};
	
	/*
	 * Add task to queue.
	 * 
	 * task.values — values for sending to router;
	 * task.reload — reload page after finishing request (on success);
	 * task.onSuccess — callback on success;
	 * task.kdbCmd — command for KDB. If not set — it saves values,
	 *  if set to "lrm" — removes key from KDB with name passed in parameter "item".
	 */
	this.addTask = function(task) {
		/* check if queue is blocked */
		if (block) {
			return;
		}
		
		/* add task to queue */
		queue.push(task);
		
		/* if after task's completion we need to reload page — block queue */
		if (task.reload) {
			block = true;
		}
		
		/* try to start task */
		runTask();
	};
	
	/* if nothing is running — run task from queue */
	var runTask = function() {
		updateMessage();
		if (queue.length == 1) {
			sendTask(queue[0]);
		}
	};
	
	/* run next task in queue */
	var nextTask = function() {
		updateMessage();
		if (queue.length > 0) {
			sendTask(queue[0]);
		}
	};
	
	/* send task to router */
	var sendTask = function(task) {
		var url;
		
		/* decide what to do */
		switch (task.kdbCmd) {
			case "lrm":
				url = "sh/kdb_del_list.cgi";
				break;
			case "rm":
				url = "sh/kdb_del.cgi";
				break;
			default:
				url = "sh/kdb_save.cgi";
				break;
		}
		
		var options = {
			"url": url,
			"type": "POST",
			"data": task.values,
			"success": function(data, textStatus) {
				wf2Logs.addLog("AJAX task response", $.sprintf("status: %s, url: %s, data: %s",
						textStatus, this.url, this.data));

				performingTask = false;
				
				/* if task need page reloading — reload */
				if (task.reload) {
					block = false;
					document.location.reload();
				}
				
				/* remove completed task from queue */
				queue.shift();
				
				updateMessage();
				
				/* if callback is set — call it */
				if (task.onSuccess) {
					task.onSuccess();
				}
				
				/* run next task */
				nextTask();
			}
		};
		
		performingTask = true;
		
		/* perform request */
		$.ajax(options);

		wf2Logs.addLog("AJAX task request", url + ", " + task.values);
	};
}

/*
 * Cache for command's output.
 */
function CmdCache() {
	var cache = new Object();
	var callbacks = [];
	var cmds = 0;
	
	/*
	 * Run asynchronously cmd and add it's output to cache.
	 * 
	 * cmd — cmd to run;
	 * alias — by default, you get cmd output by calling getCachedOutput()
	 * with cmd as parameter, but with alias you can use it instead of cmd.
	 * This can be usefull if cmd is too long or complex.
	 */
	this.runCmd = function(cmd, alias) {
		cmds++;
		cache[alias ? alias : cmd] = "waiting result...";

		config.cmdExecute({
			"cmd": cmd,
			"callback": function(data) {
				cache[alias ? alias : cmd] = data;
				cmds--;
				
				/* run callbacks when all cmds are finished */
				if (cmds <= 0) {
					/* callback can add another cmds and callbacks, so fix number of current callbacks */
					var callbacksNum = callbacks.length;
					for (var i = 0; i < callbacksNum; i++) {
						callbacks[i]();
					}
					/* remove only called callbacks */
					callbacks.splice(0, callbacksNum);
				}
			}
		});
	};
	
	/*
	 * Get output of cmd.
	 */
	this.getCachedOutput = function(cmd) {
		return cache[cmd];
	};
	
	/*
	 * Add callback to call when all backgrounded requests will be finished.
	 */
	this.onFinish = function(callback) {
		/* if there is no cmds, run callback immediately */
		if (cmds <= 0) {
			callback();
		} else {
			callbacks.push(callback);
		}
	};
}

/*
 * Local config.
 */
function Config() {
	var online = true;
	var outer = this;
	var offlineMessage = _("Router is OFFLINE! Check that router is available via your network.");
	var kdbQueue = new KDBQueue();
	var cmdCache = new CmdCache();
	
	this.isOnline = function() {
		return online;
	};
	
	/*
	 * Submit task for execution.
	 * 
	 * fields — array with fields
	 * (e.g., [ { name: 'username', value: 'jresig' }, { name: 'password', value: 'secret' } ]).
	 * reload — reload page after request is finished.
	 * onSuccess — callback on request success.
	 */
	this.kdbSubmit = function(fields, reload, onSuccess) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		this.saveVals(fields);
		
		/* encode fields with $.param() */
		kdbQueue.addTask({"values": $.param(fields), "reload": reload, "onSuccess": onSuccess});
	};
	
	/*
	 * Delete list key.
	 * 
	 * item — key to delete.
	 * subsystem — subsystem to restart.
	 */
	this.kdbDelListKey = function(item, subsystem) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		/* delete item for local KDB */
		this.delListKey(item);
		
		/* create fields for sending to router */
		fields = [
			{"name": "item", "value": item},
			{"name": "subsystem", "value": subsystem}
		];
		
		/* encode fields with $.param(), and set kdbCmd to "lrm" */
		kdbQueue.addTask({"values": $.param(fields), "kdbCmd": "lrm"});
	};
	
	/*
	 * Save values to local KDB.
	 */
	this.saveVals = function(fields) {
		$.each(fields, function(num, field) {
			outer.conf[field['name']] = field['value'];
		});
	};
	
	/*
	 * Return KDB key's value with replaced special characters.
	 */
	this.get = function(name) {
		return this.conf[name] != undefined ? this.conf[name] : null;
	};
	
	/*
	 * Return OEM key's value with replaced special characters.
	 */
	this.getOEM = function(name) {
		return this.oem[name] != undefined ? this.oem[name] : null;
	};
	
	/*
	 * Return key's parsed value. Always returns array, even there is no such key.
	 * If key's string ends with "*", returns array with keys, where "*" replaced with number
	 * (this method is faster than getByRegexp).
	 * 
	 * name — field's name.
	 */
	this.getParsed = function(key) {
		/* if key have mask */
		if (key.search(/\*$/) != -1) {
			var values = new Object();
			key = key.replace(/\*$/g, "");
			for (var i = 0; ; i++) {
				/* add number to an end of key */
				var curKey = key + i;
				if (this.conf[curKey] != undefined) {
					values[curKey] = this.parseRecord(this.conf[curKey]);
				} else {
					return values;
				}
			}
		}
		return this.conf[key] != undefined ? this.parseRecord(this.conf[key]) : new Array();
	};
	
	/*
	 * Returns object with keys, that match the regexp.
	 * 
	 * regexp — regexp to match;
	 * parse — if true, returns parsed values, otherwise returns raw values.
	 */
	this.getByRegexp = function(regexp, parse) {
		var result = {};
		$.each(this.conf, function(key, value) {
			if (regexp.test(key)) {
				result[key] = parse ? outer.parseRecord(value) : value;
			}
		});
		return result;
	};
	
	/*
	 * Deletes key from local KDB.
	 */
	this.del = function(key) {
		delete this.conf[key];
	};
	
	/*
	 * Deletes keys from local KDB by regexp.
	 */
	this.delByRegexp = function(regexp) {
		$.each(this.conf, function(key, value) {
			if (regexp.test(key)) {
				delete outer.conf[key];
			}
		});
	};
	
	/*
	 * Delete list key from local KDB.
	 */
	this.delListKey = function(key) {
		/* delete key from local KDB */
		this.del(key);
		
		/* remove key list's index */
		key = key.replace(/[0-9]+$/g, "");
		
		/* rename remaining keys */
		for (var oldIdx = 0, newIdx = 0; ; oldIdx++) {
			if (this.conf[key + oldIdx] != undefined) {
				this.conf[key + newIdx] = this.conf[key + oldIdx];
				newIdx++;
			/* 
			 * if next index is also undefined and we've deleted key not 
			 * from the end — delete last key, because we copied it to previous index.
			 */
			} else if (this.conf[key + (oldIdx + 1)] == undefined && oldIdx != newIdx) {
				delete this.conf[key + (oldIdx - 1)];
				break;
			}
		}
	};
	
	/*
	 * Parse data with format:
	 *  sys_pcitbl_s0004_ifnum=2\n
     *  sys_pcitbl_s0004_iftype=mr17h
	 * 
	 * Decodes KDB special characters.
	 * Returns hash with parsed data.
	 * 
	 * data — data to parse.
	 */
	this.parseData = function(data) {
		var config = new Object();
		
		var lines = data.split("\n");
		$.each(lines, function(name, line) {
			if (line == "KDB" || line.length == 0) return true;
			var record = line.split("=");
			if (record.length > 1) {
				/* remove double qoutes " at beginning and end of the line */
				var value = record[1].replace(/^"/g, "");
				value = value.replace(/"$/g, "");
				
				/* decode space,=,#, encoded by KDB's export */
				config[record[0]] = outer.replaceSpecialChars(value);
			}
		});
		
		return config;
	};
	
	/* 
	 * Parse record from KDB. If it consist of several variables — return array.
	 * Variables are separated by '\040', space character or by '\n'.
	 * 
	 * record — record to parse.
	 */
	this.parseRecord = function(record) {
		var parsedRecord = new Array();
		
		if (record.length == 0) return parsedRecord;
		
		/* \040 is a " " symbol. split records by space or new line */
		var variableSet = record.split(/\\040|\\n| /);
		
		/* if we have single variable in the record, add it to the array and return */
		if (variableSet.length == 1) {
			parsedRecord.push(record);
			return parsedRecord;
		}
		
		/* parse every variable in record */
		$.each(variableSet, function(name, value) {
			/* \075 is a "=" symbol */
			var variable = value.split(/\\075|=/);
			/* if we have only value */
			if (variable.length == 1) {
				parsedRecord.push(value);
			/* if we have key and value like key=value */
			} else {
				parsedRecord[variable[0]] = variable[1];
			}
		});
		return parsedRecord;
	};
	
	/*
	 * Replaces special KDB characters with next characters:
	 * \040 — ' ';
	 * \075 — '=';
	 * \043 — '#';
	 */
	this.replaceSpecialChars = function(value) {
		value = value.replace(/\\040|\\n/g, " ");
		value = value.replace(/\\043/g, "#");
		return value.replace(/\\075/g, "=");
	};
	
	/*
	 * Load KDB file from router.
	 */
	this.loadKDB = function() {
		this.conf = this.parseData($.ajax({
			type: "GET",
			url: "sh/kdb_load.cgi",
			dataType: "text",
			async: false
		}).responseText);
	};
	
	/*
	 * Load OEM file from router.
	 */
	this.loadOEM = function() {
		this.oem = this.parseData($.ajax({
			type: "GET",
			url: "sh/oem_load.cgi",
			dataType: "text",
			async: false
		}).responseText);
	};
	
	/*
	 * Updates specified key's values from router's KDB.
	 * 
	 * keys — keys to update.
	 */
	this.updateValues = function(keys) {
		/* prepare KDB command */
		var kdbArg = "";
		$.each(keys, function(num, key) {
			kdbArg += $.sprintf("get %s : ", key);
		});
		
		/* execute command */
		var newVals = config.cmdExecute({
			"cmd": $.sprintf("/usr/bin/kdb %s", kdbArg),
			"async": false
		}).split("\n");
		
		/* update keys in local KDB with new values */
		var conf = this.conf;
		$.each(keys, function(num, key) {
			conf[key] = newVals[num];
		});
	};
	
	/*
	 * Update list of network interfaces.
	 * Private function.
	 */
	var updateIfaces = function() {
		/* get "valid" records for interfaces */
		var validIfaces = config.getByRegexp(/(sys_iface_)*(_valid)/);
		
		/* create sorted array with interfaces */
		var ifaces = new Array();
		$.each(validIfaces, function(key, value) {
			/* push to array only interface name */
			key = key.replace(/sys_iface_/, "");
			ifaces.push(key.replace(/_valid\w*/, ""));
		});
		ifaces = $.unique(ifaces);
		ifaces.sort();
		ifaces = ifaces.toString().replace(/,/g, " ");

		/* create data for submission */		
		var ifacesProp = new Array();
		$.addObjectWithProperty(ifacesProp, "sys_ifaces", ifaces);
		outer.kdbSubmit(ifacesProp);
		
		/* update menu */
		generateMenu();
	};
	
	/*
	 * Add new interface to KDB.
	 * 
	 * options — interface parameters.
	 * return new interface name.
	 */
	this.addIface = function(options) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		/* return next interface name for given protocol */
		var getNextIface = function(proto) {
			/* replace protocol with inteface name */
			proto = proto.replace("bonding", "bond");
			proto = proto.replace("bridge", "br");
			
			/* find max interface index */
			var maxIdx = -1;
			var ifaces = outer.getParsed("sys_ifaces");
			$.each(ifaces, function(num, iface) {
				if (iface.search(proto) != -1) {
					var idx = parseInt(iface.replace(proto, ""), 10);
					if (idx > maxIdx) {
						maxIdx = idx;
					}
				}
			});
			
			/* return next interface's name */
			return proto + (maxIdx + 1);
		};
		
		/* get interface name */
		var iface = options['iface'] ? options['iface'] : getNextIface(options['proto']);
		
		/* create interface parameters */
		var ifaceProp = [];
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_proto", iface), options['proto']);
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_real", iface),
			options['real'] ? options['real'] : iface);
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_depend_on", iface),
			options['dependOn'] ? options['dependOn'] : "none");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_valid", iface), "1");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_auto", iface), "0");
		$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_method", iface), "none");
		if (options['vlanId']) {
			$.addObjectWithProperty(ifaceProp, $.sprintf("sys_iface_%s_vlan_id", iface),
				options['vlanId']);
		}
		
		this.kdbSubmit(ifaceProp);
		updateIfaces();

		/* return interface name */
		return iface;
	};
	
	/*
	 * Delete network interface from KDB.
	 * 
	 * iface — interface to delete.
	 */
	this.delIface = function(iface) {
		/* if router is offline, show error message and do nothing */
		if (!online) {
			alert(offlineMessage);
			return;
		}
		
		/* delete all interface parameters from local KDB */
		this.delByRegexp(new RegExp($.sprintf("sys_iface_%s_\w*", iface)));
		
		/* create data for submission */		
		var submitData = new Array();
		$.addObjectWithProperty(submitData, "item", $.sprintf("sys_iface_%s_*", iface));
		$.addObjectWithProperty(submitData, "subsystem", $.sprintf("iface_del.%s", iface));
		
		/* encode data with $.param(), and set kdbCmd to "rm" */
		kdbQueue.addTask({"values": $.param(submitData), "kdbCmd": "rm"});
		updateIfaces();
	};
	
	this.runCmd = cmdCache.runCmd;
	
	this.getCachedOutput = cmdCache.getCachedOutput;
	
	this.onCmdCacheFinish = cmdCache.onFinish;
	
	/*
	 * Start checking for router state (online or offline) every timeout seconds.
	 * If router is offline, show OFFLINE message above the menu.
	 */
	this.startCheckStatus = function(timeout) {
		var checkStatus = function() {
			var options = {
				"type": "POST",
				"url": "sh/execute.cgi",
				"dataType": "text",
				"data": {"cmd": "/bin/true"},
				"timeout": timeout * 1000,
				"success": function(data) {
					if (!online) {
						online = true;
						$("#status_state").text(_("online")).removeClass("error");
					}
				},
				"error": function() {
					online = false;
					if (ajaxNum > 0) {
						ajaxNum = 0;
						$("#status_ajax").text("requests are lost");
					}
					if (kdbQueue.getTasksNumber() > 0) {
						document.write(
							$.sprintf("<html><head><title>%s</title></head><body><h2>%s</h2><br><a href='/wf2/'>%s</a></body></html>",
								_("Connection error"),
								_("Connection error while performing tasks on a device. Check your connection to the device and reload web-interface. All unfinished tasks are lost."),
								_("Reload web-interface"))
						);
						document.close();
					} else $("#status_state").text(_("offline")).addClass("error");
				}
			};
			$.ajax(options);
		}
		
		setInterval(checkStatus, timeout * 1000);
	};
	
	/*
	 * Do Ajax request for command execution.
	 * If router is offline, result is "Router is offline".
	 * Returns command's output (for sync request).
	 * 
	 * options.cmd — command to execute;
	 * options.container — set html of container to command's output;
	 * options.callback — function to call after request. filtered command's output is passed to func as arg;
	 * options.async — sync/async request (by default request is ASYNC);
	 * options.dataType — if not specified, "text" is used;
	 * options.filter — function to filter command's output. command's output is passed to func as arg;
	 * options.formatData — if true, formats data for correct output in browser.
	 */
	/* number of performing requests */
	var ajaxNum = 0;
	this.cmdExecute = function(options) {
		var output = null;
		
		/* process Ajax result */
		var processResult = function(data) {
			/* save data to output */
			output = data;
			
			/* set data to conainer */
			if (options.container) {
				$(options.container).html(data);
				
				/* workaround for max-height in IE */
				$(options.container).minmax();
			}
			
			/* call callback with data as argument */
			if (options.callback) {
				options.callback(data);
			}
		};
		
		/* set AJAX options */
		var ajaxOptions = {
			"type": "POST",
			"url": "sh/execute.cgi",
			"async": options.async != undefined ? options.async : true,
			"dataType": options.dataType != undefined ? options.dataType : "text",
			"data": {"cmd": options.cmd},
			"dataFilter": function(data) {
				if (options.formatData == true) {
					data = data.replace(/&/g, "&amp;").replace(/ /g, "&nbsp;")
							.replace(/\t/g, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")
							.replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/\n/g, "<br>");
				}

				if (options.filter) {
					return options.filter(data);
				} else {
					return data;
				}
			},
			"success": function(data, textStatus) {
				wf2Logs.addLog("AJAX response", $.sprintf("status: %s, url: %s, cmd: %s, data: %s",
						textStatus, this.url, this.data, data));
				
				if (ajaxNum > 0) {
					ajaxNum--;
				}
				
				if (ajaxNum == 0) {
					$("#status_ajax").text(_("none"));
				} else {
					$("#status_ajax").text($.sprintf("%s (%s: %s)", _("loading data"), _("requests"),
						ajaxNum));
				}
				
				processResult(data);
			},
			"error": function(XMLHttpRequest, textStatus, errorThrown) {
				wf2Logs.addLog("AJAX error", $.sprintf("status: %s, url: %s, cmd: %s",
						textStatus, this.url, this.data));

				if (ajaxNum > 0) {
					ajaxNum--;
				}
				
				if (ajaxNum == 0) {
					$("#status_ajax").text(_("none"));
				}
				
				processResult(_("Connection error"));
			}
		};
		
		/* if router is offline show corresponding message */
		if (!config.isOnline()) {
			processResult(_("Router is offline"));
			return;
		}
		
		/* increase number of processing Ajax requests and update status */
		ajaxNum++;
		$("#status_ajax").text($.sprintf("%s (%s: %s)", _("loading data"), _("requests"), ajaxNum));
		
		/* perform request */
		$.ajax(ajaxOptions);
		
		/* log request */
		wf2Logs.addLog("AJAX request", ajaxOptions.url + ", " + options.cmd)
		
		return output;
	};
	
	var savedData = {};
	this.saveData = function(name, value) {
		savedData[name] = value;
	};
	
	this.getData = function(name) {
		return savedData[name];
	};
}
