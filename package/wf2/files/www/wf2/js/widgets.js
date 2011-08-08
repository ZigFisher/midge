/*
 * Create page element. Page consist of tabs.
 */
function Page(p) {
	/* prefix to tab's id */
	var tabIdPrefix = "tab_";
	
	/* pointer to the first link for a tab */
	var firstTab = null;
	
	/* clear container */
	$(p).empty();
	
	/* array with tabs' info */
	this.tabsInfo = [];
	
	/* common for all tabs subsystem and help */
	this.subsystem = null;
	this.helpPage = null;
	
	/* set subsystem common for all tabs */
	this.setSubsystem = function(subsystem) {
		this.subsystem = subsystem;
	};
	
	/* Set help page common for all tabs. Help section is set to the id of tab */
	this.setHelpPage = function(helpPage) {
		this.helpPage = helpPage;
	};
	
	/*
	 * Add tab.
	 * - tab: tab.id — id of the tab;
	 *        tab.name — name of the tab;
	 *        tab.func — function to call to generate tab's content.
	 */
	this.addTab = function(tab) {
		this.tabsInfo.push(tab);
	};
	
	/* generate tabs' links and divs */
	this.generateTabs = function() {
		/* create ul for tabs' links */
		var tabsList = $.create("ul");
		
		/* go through tabs' info */
		$.each(this.tabsInfo, function(num, tab) {
			/* create link to a tab */
			var href = $.create("a",
				{
					"href": "#" + tabIdPrefix + tab.id,
					"id": tabIdPrefix + tab.id + "_link"
				},
				$.create("span", {}, [
					$.create("img", {"src": "ui/img/agt_reload_3236_12.gif", "border": "0"}), _(tab.name)])
			);
			
			/* add click event */
			href.click(function(e) {
				$(".tabs-container").html("Generating content...");
				setTimeout(function() {
					/*
					 * Clear all tabs (to prevent problems when elements have identical IDs on
					 * different tabs).
					 */
					$(".tabs-container").empty();

                    scrollTo(0, 0);

					/* render tab's content */
					tab.func();
					
					/* save selected tab ID in cookie */
					$.cookie("wf2-tab", tab.id);
				}, 10);
			});
			
			/* save pointer to the first link for a tab */
			if (!firstTab) {
				firstTab = href;
			}
			
			/* add link to the list */
			$.create("li", {}, href).appendTo(tabsList);
		});
		
		/* add list with links to a page */
		tabsList.appendTo(p);
		
		/* go through tabs' info */
		$.each(this.tabsInfo, function(num, tab) {
			/* create div for a tab */
			$.create("div", {"id": tabIdPrefix + tab.id}).appendTo(p);
		});
		
		/* update tabs */
		$(p).tabs({"fxAutoHeight": true});
		
		/* if tab ID is saved in cookie and this page have tab with this ID — show this tab */
		if ($.cookie("wf2-tab") && $($.sprintf("#%s%s_link", tabIdPrefix, $.cookie("wf2-tab"))).length > 0) {
			$($.sprintf("#%s%s_link", tabIdPrefix, $.cookie("wf2-tab"))).click();
		/* otherwise show first tab */
		} else {
			firstTab.click();
		}
	};
	
	/*
	 * Create Container for a tab.
	 *
	 * - tabId - ID of tab to create container for;
	 * - options - additional container options.
	 */
	this.addContainer = function(tabId, options) {
		var params = {"subsystem": this.subsystem, "help": {"page": this.helpPage, "section": tabId}};
		if (options) {
			params = $.extend(params, options);
		}
		
		return new Container($("#" + tabIdPrefix + tabId), params);
	};

	/*
	 * Returns raw div for specified tab.
	 *
	 * - tabId — ID of tab.
	 */
	this.getRaw = function(tabId) {
		return $("#" + tabIdPrefix + tabId);
	};
	
	/* Add line break to the tab */
	this.addBr = function(tabId) {
		/* if there is no infoMessage, append line break to the tab, otherwise insert before it */
		if ($("#info_message").length == 0) {
			$.create("br").appendTo($("#" + tabIdPrefix + tabId));
		} else {
			$.create("br").insertBefore("#info_message");
		}
	};
	
	this.clearTab = function(tabId) {
		$('#' + tabIdPrefix + tabId).empty();
	};
	
	this.getTab = function(tabId) {
		return $("#" + tabIdPrefix + tabId);
	};
}

/* show HTML page in popup window */
function popup(url) {
	 var width  = 608;
	 var height = 700;
	 var left   = (screen.width - width)/2;
	 var top    = (screen.height - height)/2;
	 var params = 'width='+width+', height='+height;
	 params += ', top='+top+', left='+left;
	 params += ', directories=no';
	 params += ', location=no';
	 params += ', menubar=no';
	 params += ', resizable=no';
	 params += ', scrollbars=1';
	 params += ', status=1';
	 params += ', toolbar=no';
	 newwin = window.open(url,'help', params);
	 if (window.focus) { newwin.focus() }
	 return false;
}

/*
 * Container for widgets.
 * p — parent container.
 * options — container options (subsystem & help), set in Page object.
 * I18N for widgets.
 */
function Container(p, options) {
	/* link to this object */
	var thisContainer = this;
	
	/* ID of div for info or error messages */
	var infoMessage = "info_message";

	/* message to show on success form saving */
	var successMessage;
	
	/* set subsystem common for all tabs */
	this.subsystem = options.subsystem;
	
	/* set help page common for all tabs */
	this.help = options.help;
	
	/*
	 * Create necessary data structures and page elements.
	 */
	this.initContainer = function(options) {
		/* clear current form */
		if (options && options.clearForm) {
			this.form.empty();
			
			this.validator_rules = {};
			this.validator_messages = {};
			
			this.table = $.create("table", 
					{"id": "conttable", "cellpadding": "0", "cellspacing": "0", "border": "0"})
					.appendTo(this.form);
			this.table.append($.create("thead"));
			this.table.append($.create("tbody"));
			
			return;
		}
		
		/* clear complete tab */
		if (options && options.clear) {
			p.empty();
		}
		
		/* create, if is not exist, div for info or error messages */
		if ($("#" + infoMessage, p).length == 0) {
			$.create("div", {"className": "message", "id": infoMessage}).appendTo(p);
		}
		
		/* insert new form before div with info message */
		this.form = $.create("form", {"action": ""});
		$("#" + infoMessage, p).before(this.form);
		
		/* create table and add it to form */
		this.table = $.create("table",
				{"id": "conttable", "cellpadding": "0", "cellspacing": "0", "border": "0"})
				.appendTo(this.form);
		this.table.append($.create("thead"));
		this.table.append($.create("tbody"));

		this.validator_rules = {};
		this.validator_messages = {};
	};
	
	/* init container */
	this.initContainer(options);

	/* set subsystem for this tab */
	this.setSubsystem = function(subsystem) {
		this.subsystem = subsystem;
	};
	
	/* set help page for this tab */
	this.setHelpPage = function(helpPage) {
		this.help.page = helpPage;
	};
	
	/* set subsystem for this tab */
	this.setHelpSection = function(helpSection) {
		this.help.section = helpSection;
	};
	
	/* redraw current tab */
	this.containerRedraw = function() {
		$($.sprintf("#%s_link", p.attr("id"))).click();
	};

	this.setSuccessMessage = function(msg) {
		successMessage = msg;
	}
	
	/* 
	 * Adds title and context help link to container and adds it to container's table.
	 * 
	 * title — I18N title;
	 * options.colspan — number of cols to span for title cell (default is 2);
	 * options.internal — place title inside table (add row to table's body);
	 * options.noHelp — do not show link for context-help;
	 * options.help — use this context-help settings instead of container's.
	 */
	this.addTitle = function(title, options) {
		var colspan = options && options.colspan ? options.colspan : "2";
		var url = null;
		
		/* if we have context-help and options.noHelp is not set — add links to context-help */
		if (config.getCachedOutput("context-help") == "1" &&
			(!options || (options && !options.noHelp))) {
			/* define which parameter to use */
			var help = options && options.help ? options.help : this.help;
			
			/* add context-help link */
			if (help.page && help.section) {
				url = $.sprintf("/help/%s.html#%s", help.page, help.section);
			} else if (help.page) {
				url = $.sprintf("/help/%s.html", help.page);
			}
		}
		
		/* if url is set — create context help link object, otherwise set it to null */
		var helpLink = url ? $.create("a", {"href": "#", "className": "helpLink"}, "[?]")
				.click(function() {
					popup(url);
				}) : null;
		
		/* title is placed inside table */
		if (options && options.internal) {
			var tr = $.create("tr", {"align": "center"});
			$.create("td", {"colSpan": colspan, "className": "header"}, [_(title), " ", helpLink])
					.appendTo(tr);
			$("tbody", this.table).append(tr);
		/* title is placed in the header section of the table */
		} else {
			/* create table's row for title and context help link */
			$("thead", this.table).append(
				$.create("tr", {},
					$.create("th", {"colSpan": colspan}, [_(title), " ", helpLink])
				)
			);
		}
	};
	
	/*
	 * Create general data for all widgets elements.
	 * I18N for text and description.
	 * 
	 * w — widget's info.
	 */
	this.createGeneralWidget = function(w) {
		/* if this field is required — show "*" */
		var required = (w.validator && w.validator.required) ? " *" : "";
	
		/* if description is specified — show it */
		var tdElements;
		if (w.descr) {
			tdElements = [$.create("br"), $.create("p", {}, _(w.descr))];
		}
		
		/* create table row for widget */
		var tr = $.create("tr", {}, [
				$.create("td", {"className": "tdleft"}, _(w.text) + required),
				$.create("td", {"id": "td_" + w.name}, tdElements)
			]
		);
		
		return tr;
	};
	
	/*
	 * Create text widget.
	 * I18N for tip.
	 */
	this.createTextWidget = function(w, value) {
		var attrs = {
			"type": "text",
			"name": w.name,
			"id": w.id
		};
		w.tip && (attrs.title = _(w.tip));
		
		/* set KDB value */
		if (value) {
			attrs.value = value;
		/* if KDB value does't exists — set default value, if it exists */
		} else if (w.defaultValue != undefined) {
			attrs.value = w.defaultValue;
		}
		
		return $.create("input", attrs);
	};
	
	/*
	 * Create password widget.
	 * I18N for tip.
	 */
	this.createPasswordWidget = function(w, value) {
		var attrs = {
			"type": "password",
			"name": w.name,
			"id": w.id
		};
		w.tip && (attrs.title = _(w.tip));
		
		/* set KDB value */
		if (value) {
			attrs.value = value;
		}
		
		return $.create("input", attrs);
	};
	
	/*
	 * Create checkbox widget.
	 * I18N for tip.
	 */
	this.createCheckboxWidget = function(w, value) {
		var attrs = {
			"type": "checkbox",
			"name": w.name,
			"id": w.id,
			"className": "check",
			"value": "1"
		};
		w.tip && (attrs.title = _(w.tip));
		if (value) {
			if (value == "1" || value == "on") {
				attrs.checked = true;
			}
		} else if (w.defaultState == "checked") {
			attrs.checked = true;
		}
		
		return $.create("input", attrs);
	};
	
	/*
	 * Create select widget.
	 * I18N for tip.
	 *
	 * w.addCurrentValue - if current value is not in select's options list, add it and select.
	 */
	this.createSelectWidget = function(w) {
		var attrs = {
			"name": w.name,
			"id": w.id
		};
		w.tip && (attrs.title = _(w.tip));
		
		return $.create("select", attrs);
	};
	
	/*
	 * Add HTML text.
	 * 
	 * w.dataFilter — may be used with w.cmd — function to filter command's output data.
	 */
	this.createHtmlWidget = function(w, value) {
		var attrs = {"className": "htmlWidget"};
		w.tip && (attrs.title = _(w.tip));
		
		var span = $.create("span", attrs);
		if (w.kdb != undefined) {
			$(span).html(value);
		} else if (w.cmd != undefined) {
			$(span).html("Loading...");
			if (w.dataFilter) {
				config.cmdExecute({"cmd": w.cmd, "container": span, "filter": w.dataFilter});
			} else {
				config.cmdExecute({"cmd": w.cmd, "container": span});
			}
		} else if (w.str != undefined) {
			$(span).html(w.str);
		}
		
		return span;
	};

	/*
	 * Create hidden widget.
	 */
	this.createHiddenWidget = function(w, value) {
		var attrs = {
			"type": "hidden",
			"name": w.name,
			"id": w.id
		};
		
		/* set KDB value */
		if (value) {
			attrs.value = value;
		/* if KDB value does't exists — set default value, if it exists */
		} else if (w.defaultValue != undefined) {
			attrs.value = w.defaultValue;
		}
		
		return $.create("input", attrs);
	};
	
	/*
	 * Create file widget.
	 */
	this.createFileWidget = function(w) {
		var attrs = {
			"type": "file",
			"name": w.name,
			"id": w.id
		};
		
		return $.create("input", attrs);
	};

	/*
	 * Create button with click event handler.
	 * I18N for text and tip.
	 */
	this.createButtonWidget = function(w) {
		var attrs = {
			"type": "button",
			"name": w.name,
			"id": w.id,
			"value": _(w.text),
			"className": "buttonWidget"
		};
		w.tip && (attrs.title = _(w.tip));
		
		var button = $.create("input", attrs);
		
		/* set action */
		button.click(function(src) {
			if (w.eventHandlerObject) {
				w.func(w.eventHandlerObject, src);
			} else {
				w.func(thisContainer, src);
			}
		});
		
		return button;
	};

	/*
	 * Place element (widget or subwidget) depending on it's type and placement option.
	 * 
	 * element — jQuery element to place;
	 * type — type of element (widget/subwidget):
	 *  if placement is not set:
	 *   widget element is appended to the end of a table;
	 *   subwidget element is added to it's widget;
	 * name — name property of element (w.name);
	 * placement — optional placement strategy:
	 *  - placement.type — where to place element;
	 *  - placement.anchor — another element which is used for placing this element.
	 */
	var placeElement = function(element, type, name, placement) {
		if (placement) {
			switch (placement.type) {
				case "insertAfter":
					if (placement.anchor == null || placement.anchor == undefined) {
						throw "placement.anchor is not specified or invalid";
					}
					element.insertAfter(placement.anchor);
					break;
				case "insertBefore":
					if (placement.anchor == null || placement.anchor == undefined) {
						throw "placement.anchor is not specified or invalid";
					}
					element.insertBefore(placement.anchor);
					break;
				case "prependToAnchor":
					if (placement.anchor == null || placement.anchor == undefined) {
						throw "placement.anchor is not specified or invalid";
					}
					element.prependTo(placement.anchor);
					break;
				case "appendToAnchor":
					if (placement.anchor == null || placement.anchor == undefined) {
						throw "placement.anchor is not specified or invalid";
					}
					element.appendTo(placement.anchor);
					break;
				case "appendToForm":
					thisContainer.form.append(element);
					break;
				case "appendToTable":
					$("tbody", thisContainer.table).append(element);
					break;
				default:
					throw "placement.type is not specified or unsupported";
			}
		} else {
			/* by default, append widget to table */
			if (type == "widget") {
				$("tbody", thisContainer.table).append(element);
			/* by default, add subwidget to it's widget */
			} else {
				$("#td_" + name, thisContainer.form).prepend(element);
			}
		}
	};

	/*
	 * Add complete widget (table's TR) to container.
	 * If type is general, add only table's TR.
	 * 
	 * w — widget to add;
	 * placement — how to place widget (look at placeElement());
	 */
	this.addWidget = function(w, placement) {
		if (w.type != "hidden") {
			placeElement(this.createGeneralWidget(w), "widget", w.name, placement);
			if (w.type == "general") {
				return;
			}
		}
		
		this.addSubWidget(w);
	};
	
	/*
	 * Add subwidget (pure HTML element: input, select, etc) to it's widget or specified position.
	 * 
	 * w — widget to add;
	 * placement — how to place subwidget (look at placeElement()).
	 * 
	 * return added widget.
	 */
	this.addSubWidget = function(w, placement) {
		var value, subwidget;
		
		/* TODO: make full support for cookies (do not write it to KDB but save in cookie) */
		/* get field's value */
		if (w.cookie) {
			value = $.cookie(w.name);
		} else if (w.name) {
			value = w.item ? config.getParsed(w.item)[w.name] : config.get(w.name);
		}

		/* if valueFilter is set */
		if (w.valueFilter) {
			value = w.valueFilter(value);
		}
		
		/* if ID is not set, set it to the name */
		if (w.id == undefined) {
			w.id = w.name;
		}

		if (w.tip) {
			w.tip = "|" + w.tip;
		}
		
		/* create subwidget of specified type */
		switch (w.type) {
			case "text": 
				subwidget = this.createTextWidget(w, value);
				break;
			case "hidden":
				subwidget = this.createHiddenWidget(w, value);
				break;
			case "password": 
				subwidget = this.createPasswordWidget(w, value);
				break;
			case "checkbox":
				subwidget = this.createCheckboxWidget(w, value);
				break;
			case "html":
				subwidget = this.createHtmlWidget(w, value);
				break;
			case "file":
				subwidget = this.createFileWidget(w);
				break;
			case "select":
				subwidget = this.createSelectWidget(w);

				/* add list's options */
				if (w.options) {
					subwidget.setOptionsForSelect({
							"options": w.options,
							"curValue": value,
							"defaultValue": w.defaultValue
					});
				}

				/* if addCurrentValue is set and this value is not in options list, add it and select */
				if (value && w.addCurrentValue
						&& $($.sprintf("option[value=%s]", value), subwidget).length == 0) {
					subwidget.setOptionsForSelect({
							"options": value,
							"curValue": value,
							"doNotClear": true
					});
				}
				
				break;
			case "button":
				subwidget = this.createButtonWidget(w);
				break;
			default:
				throw "unknown widget type";
		}
		
		/* subwidgets of hidden type are always added to the end of a form */
		if (w.type == "hidden") {
			this.form.append(subwidget);
		/* other subwidgets are placed depending on their placement property */
		} else {
			placeElement(subwidget, "subwidget", w.name, placement);
		}
		
		/* set CSS classes if specified. they can be space-separated */
		if (w.cssClass) {
			$.each(w.cssClass.split(" "), function(num, cssClass) {
				subwidget.addClass(cssClass);
			});
		}
		
		/* set nice tooltip */
		subwidget.tooltip({"track": true, "showBody": "|"});
		
		/* bind specified events */
		this.bindEvents(w);
		
		w.validator && (this.validator_rules[w.name] = w.validator);
		
		/* I18N for element's error messages */
		w.message && (this.validator_messages[w.name] = _(w.message));
		
		return subwidget;
	};
	
	/*
	 * Bind events to widget.
	 * If w.eventHandlerObject is set, then pass it as parameter to event handler,
	 * otherwise pass current container.
	 * Always pass event source as second parameter.
	 */
	this.bindEvents = function(w) {
		if (w.onChange) {
			$("#" + w.id).change(function(src) {
				return w.onChange(w.eventHandlerObject ? w.eventHandlerObject : thisContainer, src);
			});
		}
		
		if (w.onClick) {
			$("#" + w.id).click(function(src) {
				return w.onClick(w.eventHandlerObject ? w.eventHandlerObject : thisContainer, src);
			});
		}

		if (w.onKeydown) {
			$("#" + w.id).keydown(function(src) {
				return w.onKeydown(w.eventHandlerObject ? w.eventHandlerObject : thisContainer, src);
			});
		}

		if (w.onKeypress) {
			$("#" + w.id).keypress(function(src) {
				return w.onKeypress(w.eventHandlerObject ? w.eventHandlerObject : thisContainer, src);
			});
		}
	};

	/* 
	 * Sets error message.
	 * I18N for text.
	 */
	this.setError = function(text) {
		var idInfoMessage = "#" + infoMessage;
		$(idInfoMessage).html(_(text));
		$(idInfoMessage).removeClass("success_message");
		$(idInfoMessage).addClass("error_message");
	};
	
	/* 
	 * Set info message.
	 * I18N for text.
	 */
	this.setInfo = function(text) {
		var idInfoMessage = "#" + infoMessage;
		$(idInfoMessage).html(_(text));
		$(idInfoMessage).removeClass("error_message");
		$(idInfoMessage).addClass("success_message");
	};
	
	/* Show message. After clicking on widgets remove info message. */
	this.showMsg = function() {
		$("#" + infoMessage).show();

		/* set event handlers to remove info message */
		$("input, select").bind("click.tmp", function() {
			thisContainer.hideMsg();

			/* remove alert text */
			$(".alertText").remove();

			/* remove class indicating field updation */
			$("*").removeClass("fieldUpdated");

			/* remove all events handlers */
			$("input, select").unbind("click.tmp");
		});
	};
	
	/* hide message */
	this.hideMsg = function() {
		$("#" + infoMessage).hide();
	};
	
	/*
	 * Show to user that data is saved.
	 */
	var formSaved = function() {
		var msg = _("Data saved or this task is added to queue.");
		if (successMessage) {
			msg += "<br>" + _(successMessage);
		}
		thisContainer.setInfo(msg);
		thisContainer.showMsg();
	};
	
	/*
	 * If the router is offline, show message and returns false, otherwise returns true.
	 */
	var isRouterOffline = function() {
		if (!config.isOnline()) {
			thisContainer.setError("Router is OFFLINE! Check that router is available via your network.");
			thisContainer.showMsg();
			return true;
		}
		return false;
	};

	/*
	 * Adds submit button, form validation rules and submit's events handlers.
	 * 
	 * options.reload — reload page after AJAX request (e.g., for update translation);
	 * options.onSuccess — callback on request successfully completion;
	 * options.noSubmit — do not submit the form, but call onSubmit and preSubmit callbacks;
	 * options.onSubmit — callback on submit event to call after submitting the form;
	 * options.preSubmit — callback on submit event to call before submitting the form;
	 * options.additionalKeys — additional keys to save in KDB in the correct format
	 *  ([ { name: 'username', value: 'jresig' }, { name: 'password', value: 'secret' } ]).
	 * NOTE:
	 *  - if preSubmit returns false, form is not submitting;
	 *  - if noSubmit is set and preSubmit return false, onSubmit is not calling.
	 */
	this.addSubmit = function(options) {
		var idInfoMessage = "#" + infoMessage;
		var outer = this;

		/* if subsystem is set — add it to the form */
		if (this.subsystem) {
			this.form.append(
				$.create("input",
					{"type": "hidden", "name": "subsystem", "id": "subsystem", "value": this.subsystem}
				)
			);
		}

		/* create submit button */
		$.create("input", {
			"type": "submit",
			"className": "button",
			"value": options && options.submitName ? _(options.submitName) : _("Save")
		}).appendTo(this.form);
		
		if (options && options.extraButton) {
			var button = $.create("input", {
				"type": "button",
				"value": _(options.extraButton.name)
			}).appendTo(this.form);
			button.click(options.extraButton.func);
		}
		
		/* options for form validation */
		var validateOptions = {
			"onfocusout": false,
			"onkeyup": false,
			"onclick": false,
			"rules": this.validator_rules,
			"messages": this.validator_messages,
			
			/* container where to show error */
			"errorContainer": idInfoMessage,
			
			/* Set error text to container */
			"showErrors": function(errorMap, errorList) {
				thisContainer.setError("Please, enter a valid data into the form below to be able to save it successfully.");
				this.defaultShowErrors();
			},
     		
     		/* on submit event */
     		"submitHandler": function(form) {
     			/* if router is offline — return */
     			if (isRouterOffline()) {
     				return;
     			}
     			
     			/* if noSubmit is set — do not submit the form */
     			if (options && options.noSubmit) {
     				if (options.preSubmit) {
     					if (options.preSubmit() == false) {
     						return;
     					}
     				}
     				
     				if (options.onSubmit) {
     					if (options.onSubmit() == false) {
     						return;
     					}
     				}
     				
     				formSaved();
     				
     				return;
     			}
     			
     			/* call user function on submit event before submitting form */
     			if (options && options.preSubmit) {
     				if (options.preSubmit() == false) {
     					return;
     				}
     			}
     			
     			/* remove alert text */
				$(".alertText", form).remove();
				
				/* remove class indicating field updation */
				$("*", form).removeClass("fieldUpdated");
		
     			/*
     			 * All checkboxes return values, even they are unchecked.
     			 * Here we find all unchecked checkboxes, temporarily check them, set
     			 * their value, and set their class to doUncheck, to uncheck them later.
     			 */
     			$(":checkbox").not(":checked").each(function() {
					this.checked = true;
					this.value = 0;
				}).addClass("doUncheck");
				
				var reload = (options && options.reload) ? true : false;
				var onSuccess = (options && options.onSuccess) ? options.onSuccess : false;
				
				/* array with fields for saving */
				var fields;
				
				/*
				 * complexValue is name of variable, which value consist of other variables
				 * (e.g., complexValue is sys_voip_route_15, and it's value will be
				 * "enabled=1 router_id=135")
				 */
				if (options && options.complexValue) {
					var bigValue = "";
					var subsystem;

					/* go through form's fields and create complex value */
					$.each($(form).formToArray(), function(num, field) {
						/* write subsystem in separate variable */
						if (field['name'] == "subsystem") {
							subsystem = field['value'];
						/* add variable and it's value to complex value */
						} else {
							bigValue += field['name'] + "=" + field['value'] + " ";
						}
					});
					bigValue = $.trim(bigValue);

					/* create array for fields */
					fields = [];
					
					/* add complex value */
					fields.push({
						"name": options.complexValue,
						"value": bigValue
					});
					
					/* if subsystem is set, add it to array with fields */
					if (subsystem) {
						fields.push({
							"name": "subsystem",
							"value": subsystem
						});
					}
				/* if we are not in need of complex value, create array with form's fields */
				} else {
					fields = $(form).formToArray();
				}
				
				/* if additionalKeys is specified — add them */
				if (options && options.additionalKeys) {
					fields = $.merge(fields, options.additionalKeys);
				}
				
				/* submit task (updating settings) for execution */
     			config.kdbSubmit(fields, reload, onSuccess);
     			
     			formSaved();
				
				/* set checkboxes to their original state */
				$(".doUncheck").each(function() {
					this.checked = false;
					this.value = 1;
				}).removeClass("doUncheck");
				
				/* call user function on submit event */
     			if (options && options['onSubmit']) {
     				options['onSubmit']();
     			}
     		}
		};
		
		/* if widgets are placed in a table */
		if (this.isTable) {
			/* show errors for fields in the error's container above table */
			validateOptions['errorLabelContainer'] = idInfoMessage;
     		
     		/* wrap errors in "li" elements */
     		validateOptions['wrapper'] = "li"
		} else {
			validateOptions['errorPlacement'] = function(error, element) {
     			error.prependTo(element.parent());
     		}
		}
		
		/* apply validate rules to form */
		$(this.form).validate(validateOptions);
	};
	
	/*
	 * Submit form in traditional way, without AJAX.
	 * 
	 * options.submitName — name of button for submitting;
	 * options.formAction — action for the form;
	 * options.encType — enctype property for the form.
	 */
	this.addSubmitNoAjax = function(options) {
		if (options && options.formAction) {
			this.form.attr("action", options.formAction);
		}
		
		if (options && options.method) {
			this.form.attr("method", options.method);
		}
		
		if (options && options.encType) {
			this.form.attr("enctype", options.encType);
			
			/* IE fix */
			this.form.attr("encoding", options.encType);
		}
		
		/* create submit button */
		$.create("input",
			{
				"type": "submit",
				"className": "button",
				"value": options && options.submitName ? _(options.submitName) : _("Save")
			}
		).appendTo(this.form);
	};
	
	/*
	 * Creates and returns command header and output's body.
	 */
	this.createCmdTitleAndBody = function(cmd) {
		var result = 
			[
				$.create("b", {}, cmd),
				$.create("br"),
				$.create("div", {"className": "pre scrollable"}, _("Loading..."))
			];
		return result;
	};
	
	/*
	 * Adds HTML code for command output to table.
	 */
	this.addConsoleHTML = function(cmd, p) {
		$.create("tr", {},
			$.create("td", {}, this.createCmdTitleAndBody(cmd))
		).appendTo(p);
	};
	
	/*
	 * Add output of command execution to the page.
	 * 
	 * cmd — string or array with cmds' to execute.
	 */
	this.addConsole = function(cmd) {
		var outer = this;
		
		/* adds command's HTML to the page, and makes AJAX request to the server */
		var addConsoleOut = function(num, cmd) {
			outer.addConsoleHTML(cmd, outer.table);
			config.cmdExecute({
				"cmd": cmd,
				"container": $($.sprintf("tr > td > b:contains('%s')", cmd), outer.table).nextAll("div.pre"),
				"formatData": true
			});
		};
		
		/* we can have one or several commands */
		if (typeof cmd == "object") {
			$(cmd).each(function(num, cmd) {
				addConsoleOut(num, cmd);
			});
		} else {
			addConsoleOut(0, cmd);
		}
	};
	
	/*
	 * Create div in the form and add console output to it.
	 * 
	 * cmd — command to execute.
	 */
	this.addConsoleToForm = function(cmd) {
		/* create div for command output */
		var div = $.create("div", {"className": "pre, cmdOutput"}, _("Loading...")).appendTo(this.form);
		
		config.cmdExecute({
			"cmd": cmd,
			"container": div,
			"filter": function(data) {
				return data.replace(/\n/g, "<br>");
			}
		});
	};
	
	/*
	 * Run command with specified parameters.
	 * First argument — command template (e.g., "/bin/ping -c %ARG %ARG").
	 * Next arguments — name of form's fields to pass as command arguments.
	 * E.g.: addRun("/bin/ping -c %ARG %ARG", "count", "host");
	 */
	this.addRun = function() {
		/* create submit button */
		$.create("input", {"type": "submit", "className": "button", "value": _("Run")})
				.appendTo(this.form);
		
		/* create div for command output */
		var cmdOutput = $.create("div").appendTo(this.form);
		
		var runArgs = arguments;
		var outer = this;
		$(this.form).submit(function() {
			var cmd;
			
			/* make from command template real command */
			$.each(runArgs, function(num, name) {
				if (num == 0) {
					cmd = name;
					return true;
				}
				var value = $("input[name=" + name + "]").val();
				cmd = cmd.replace("%ARG", value);
			});

			/* clear div for command output */
			cmdOutput.empty();
			
			/* add command header and body to div */
			$.each(outer.createCmdTitleAndBody(cmd), function(num, element) {
				element.appendTo(cmdOutput);
			});
			
			/* execute command */
			config.cmdExecute({
				"cmd": cmd,
				"container": $("div.pre", cmdOutput),
				"filter": function(data) {
					return data.replace(/\n/g, "<br>");
				}
			});
			
			/* prevent form submission */
			return false;
		});
	};
	
	/*
	 * Add button which executes console command.
	 * 
	 * name — I18N name of button.
	 * cmd — command to execute.
	 */
	this.addAction = function(name, cmd) {
		/* create submit button */
		$.create('input', {'type': 'submit', 'className': 'button', 'value': _(name)}).appendTo(this.form);
		
		$(this.form).submit(function() {
			/* execute command */
			config.cmdExecute({"cmd": cmd});
			return false;
		});
	};
	
	/*
	 * Add header for table.
	 * 
	 * header — separated with "|" list of table column's header.
	 */
	this.addTableHeader = function(header) {
		/* set table flag (for validation) */
		this.isTable = true;
		
		var tr = $.create("tr", {"align": "center", "className": "tableHeader"});
		$.each(header.split("|"), function(num, value) {
			$.create("th", {}, _(value)).appendTo(tr);
		});
		
		/* add to thead section of current table */
		$("thead", this.table).append(tr);
	};
	
	/*
	 * Add text to tfoot section in the table.
	 * I18N for str.
	 * 
	 * str — text to add;
	 * colspan — number of columns to span for tfoot's row.
	 */
	this.addTableTfootStr = function(str, colspan) {
		if ($("tfoot", this.table).length == 0) {
			$("thead", this.table).after($.create("tfoot"));
		}
		
		$("tfoot", this.table).append($.create("tr", {}, $.create("td", {"colSpan": colspan}, _(str))));
	};
	
	/*
	 * Adds row to the table.
	 */
	this.addTableRow = function() {
		return $.create("tr", {"align": "center"}).appendTo(this.table);
	};
	
	/*
	 * Adds table's cell with proper id.
	 * 
	 * w — widget;
	 * row — destination row.
	 */
	this.addGeneralTableWidget = function(w, row, colspan) {
		var td = $.create("td", {"id": "td_" + w.name}).appendTo(row);
		
		if (colspan) {
			td.attr("colSpan", colspan);
		}
	};
	
	/*
	 * Add widget to table.
	 * 
	 * w — widget;
	 * row — destination row;
	 * colspan — optional colspan property for widget's TD.
	 */
	this.addTableWidget = function(w, row, colspan) {
		this.addGeneralTableWidget(w, row, colspan);
		
		/* add subwidget and style it */
		this.addSubWidget(w).addClass("table");		
	};

	/*
	 * Add div with static message to the top of container before form.
	 *
	 * message - text of message.
	 */
	this.addStaticMessage = function(message) {
		$.create("div", {"className": "message staticMessage"}, _(message)).prependTo(p).show();
	};

	/*
	 * Remove divs with static message.
	 */
	this.removeStaticMessages = function() {
		$(".staticMessage", p).remove();
	};
	
	/*
	 * Adds list to a container "c". It gets list values from KDB
	 * by key "listItem*", renders list title, creates function to
	 * add and delete new elements. After adding or deleting
	 * element, it redraws the page by calling click() event on
	 * tab link.
	 * 
	 * c — container to add list to;
	 * options — object with options:
	 *  - tabId — id of tab this list is added to (is used for search tab link);
	 *  - header — array with columns' headers of the list (I18N);
	 *  - varList — array with variables' names;
	 *  - listItem — template of name for list item (e.g., sys_voip_route_);
	 *  - listTitle — title for the list (I18N);
	 * 
	 *  optional parameters:
	 *  - processValueFunc — optional callback, which can be used for editing values of item's
	 *     variable. It is called for each item's variable with hash as a parameter. Keys of hash are:
	 *      - varName - current variable name;
	 *      - varValue - value of current variable;
	 *      - keyValues - hash with all values for current key (varValue = keyValues.varName).
	 *     It has to return current variable's value;
	 *  - onAddOrEditItemRender — optional callback, which is called when page for adding or
	 *     editing list's elements is rendered. It is called with one parameter — this list;
	 *  - onAddItemRender — optional callback, which is called when page for adding
	 *     list's elements is rendered. It is called with one parameter — this list;
	 *  - onEditItemRender — optional callback, which is called when page for editing
	 *     list's elements is rendered. It is called with one parameter — this list;
	 *  - checkOnDelete — optional callback, which is called when when list's item is deleting.
	 *     Callback has to return hash with following keys:
	 *      - deleteAllowed - if true, delete is allowed, if false, delete is forbidden (impossible);
	 *      - message - custom message, showed to user;
	 *      - actionOnDelete - (optional) if delete is happens, run this callback;
	 *  - checkOnSubmit — optional callback, which is called when when list's item is adding. It is
	 *     called with parameter isAdding, which indicate current operation: adding or editing.
	 *     Callback has to return hash with following keys:
	 *      - addAllowed - if true, adding is allowed, if false - is forbidden;
	 *      - message - custom message, showed to user;
	 *  - preSubmit — optional callback, which is called before submitting add/editing form;
	 *  - showPage — function to render a page after adding/editing list's item or
	 *     clicking Back button on adding/editin page. If not set, tab's function is called.
	 *  - helpPage — help page;
	 *  - helpSection — help section;
	 *  - subsystem — subsystem;
	 *  - addMessage — title for add page (I18N);
	 *  - editMessage — title for edit page (I18N).
	 */
	var List = function(c, options) {
		var list = this;
		
		/* array with widgets for add/edit page */
		var widgets = new Array();
		
		/* array with widgets for adding page only */
		var widgetsForAdding = new Array();
		
		/* redraw page after adding/editing list item */
		var showPage = function() {
			$($.sprintf("#tab_%s_link", options.tabId)).click();
		};
		
		/* 
		 * Add/edit item.
		 * 
		 * item — if this parameter is set, then edit this item.
		 */
		var addOrEditItem = function(item) {
			/* are we adding or editing item */
			var isAdding;
			
			/* clear this container */
			c.initContainer({"clear": true});

			if (options.helpPage) {
				c.setHelpPage(options.helpPage);
			}
			
			if (options.helpSection) {
				c.setHelpSection(options.helpSection);
			}
			
			if (options.subsystem) {
				c.setSubsystem(options.subsystem);
			}

			/* decide, if we will add or edit item */
			if (!item) {
				isAdding = true;
				c.addTitle(options.addMessage || "Add");
				var values = config.getParsed(options.listItem + "*");
				item = options.listItem + $.len(values);
			} else {
				isAdding = false;
				c.addTitle(options.editMessage || "Edit");
			}
			
			/* set calculated item in options object for later use in dynamic widgets */
			options.currentItem = item;

			/* add widgets */
			$.each(widgets, function(num, widget) {
				/* item property is used for properly get the value of a widget */
				widget.widget.item = item;
				
				c.addWidget(widget.widget, widget.placement);
			});

			/* only checkOnSubmit or preSubmit is available */
			if (options.preSubmit && options.checkOnSubmit) {
				throw "Use only checkOnSubmit or preSubmit!"
			}

			/* add submit (with special for list options) */
			c.addSubmit({
				"complexValue": item,
				"submitName": "Add/Update",
				"extraButton": {
					"name": "Back",
					"func": options.showPage ? options.showPage : showPage
				},
				"onSubmit": options.showPage ? options.showPage : showPage,
				"preSubmit": function() {
					if (options.preSubmit) {
						options.preSubmit();
					} else if (options.checkOnSubmit) {
						var result = options.checkOnSubmit(isAdding);
						if (result.addAllowed == false) {
							thisContainer.setError(result.message);
							thisContainer.showMsg();

							return false;
						} else {
							return true;
						}
					}
				}
			});

			/* run callback for adding/editing page with this object as a parameter */
			if (options.onAddOrEditItemRender) {
				options.onAddOrEditItemRender(list);
			}

			/* do adding/editing specific actions */
			if (isAdding) {
				/* add widgets for adding page */
				$.each(widgetsForAdding, function(num, widget) {
					/* item property is used for properly get the value of a widget */
					widget.widget.item = item;
					
					c.addWidget(widget.widget, widget.placement);
				});
				
				/* run callback for adding page with this object as a parameter */
				if (options.onAddItemRender) {
					options.onAddItemRender(list);
				}
			} else {
				/* run callback for editing page with this object as a parameter */
				if (options.onEditItemRender) {
					options.onEditItemRender(list);
				}
			}
		};
		
		/* add list header */
		var addListHeader = function() {
			var tr = $.create("tr", {"align": "center", "className": "tableHeader"});
			
			/* add columns headers */
			var thNum = 0;
			$.each(options.header, function(num, value) {
				$.create("th", {}, _(value)).appendTo(tr);
				thNum++;
			});
			
			/* add list header with colSpan parameter */
			c.addTitle(options.listTitle, {"colspan": thNum + 2});
			
			/* create "button" for adding */
			var img = $.create("img", {"src": "ui/img/plus.gif", "alt": "add"});
			img.click(function(e) {
				addOrEditItem();
				scrollTo(0, 0);
			});
			
			/* change image when mouse is over it */
			img.hover(
				function() {
					$(this).attr("src", "ui/img/plus2.gif");
				},
				function() {
					$(this).attr("src", "ui/img/plus.gif");
				}
			);
			
			/* we use colSpan because future rows will contain buttons for editing and deleting */
			$.create("th", {"colSpan": "2"}, img).appendTo(tr);
			
			/* add to thead section of current table */
			$("thead", c.table).append(tr);
		};
		
		/*
		 * Confirm item deletion.
		 * 
		 * item — item to delete.
		 */
		var deleteConfirm = function(item) {
			/* action to perform after deleting */
			var actionOnDelete;

			/* find div for showing delete confirm message */
			var msgDiv = $("div.error_message", c.form);

			/* delete is cancelled */
			var cancelDelete = function() {
				msgDiv.hide();
				$(".selected", c.table).removeClass("selected");
			};

			/* hide global info message */
			thisContainer.hideMsg();

			if (options.checkOnDelete) {
				var result = options.checkOnDelete(item);
				actionOnDelete = result.actionOnDelete;

				if (result.deleteAllowed == false) {
					msgDiv.html(result.message + "<br>");
					msgDiv.show();

					var field = {
						"type": "button",
						"name": "ok",
						"text": "Ok",
						"cssClass": "button",
						"func": cancelDelete
					};
					c.addSubWidget(field, {"anchor":  msgDiv, "type": "appendToAnchor"});
					return;
				} else if (result.deleteAllowed == true && result.message) {
					msgDiv.html(result.message + "<br>");
				} else {
					msgDiv.html(_("Are you sure you want to delete this item?") + "<br>");
				}
			} else {
				msgDiv.html(_("Are you sure you want to delete this item?") + "<br>");
			}
			
			/* create Yes button */
			var button = $.create("input", {
				"type": "button",
				"className": "button",
				"value": _("Yes")
			}).appendTo(msgDiv);
			
			/* delete item */
			button.click(function() {
				msgDiv.hide();
				
				/* if router is offline — return */
     			if (isRouterOffline()) {
     				return;
     			}
				
				$(".selected", c.table).removeClass("selected");
				
				/* delete item and restart subsystem */
				config.kdbDelListKey(item, c.subsystem);

				if (actionOnDelete) {
					actionOnDelete();
				}
				
				/* call func after deleting (updates page) */
				options.showPage ? options.showPage() : showPage();
			});
			
			/* create No button */
			button = $.create("input", {
				"type": "button",
				"value": _("No")
			}).appendTo(msgDiv);
			
			/* cancel delete */
			button.click(cancelDelete);
	
			msgDiv.show();
			return;
		};
		
		/*
		 * Add widget to add/edit page.
		 */
		this.addWidget = function(w, placement) {
			/* save link to this object to use later in events' handlers */
			w.eventHandlerObject = list;
			
			widgets.push({"widget": w, "placement": placement});
		};
		
		/*
		 * Add widget to ADD page only.
		 */
		this.addWidgetForAdding = function(w, placement) {
			/* save link to this object to use later in events' handlers */
			w.eventHandlerObject = list;
			
			widgetsForAdding.push({"widget": w, "placement": placement});
		};
		
		/*
		 * Add widget to add/edit page after it generation (e.g., in event's handler).
		 */
		this.addDynamicWidget = function(w, placement) {
			/* save link to this object to use later in events' handlers */
			w.eventHandlerObject = list;
			
			w.item = options.currentItem;
			c.addWidget(w, placement);
		};

		/*
		 * Add subwidget to add/edit page after it generation (e.g., in event's handler).
		 */
		this.addDynamicSubWidget = function(w, placement) {
			/* save link to this object to use later in events' handlers */
			w.eventHandlerObject = list;

			w.item = options.currentItem;
			c.addSubWidget(w, placement);
		};
		
		/*
		 * Generate list.
		 */
		this.generateList = function() {
			/* add list header */
			addListHeader();
			
			/* get list of items */
			var items = config.getParsed(options.listItem + "*");
			
			/* go through item's list */
			$.each(items, function(key, value) {
				var row = c.addTableRow();
				var cssClass = {};
				
				/* change text color for disabled items */
				if (value.enabled != undefined && (value.enabled != "on" && value.enabled != "1")) {
					cssClass.className = "disabled";
				}
				
				/* for each variable in item's value create table cell with variable's value */
				$.each(options.varList, function(num, variable) {
					var finalVal = options.processValueFunc
							? options.processValueFunc({
									"varName": variable,
									"varValue": value[variable],
									"keyValues": value})
							: value[variable];
					
					/* create td */
					var td = $.create("td", cssClass, finalVal ? finalVal : "&nbsp;")
						.appendTo(row);
	
					/* if function is set for this variable */
					if (options.varFunctions && options.varFunctions[variable]) {
						/* add clickable class */
						td.addClass("clickable");
						
						/* if set tip — add it */
						if (options.varFunctions[variable].tip) {
							td.attr("title", options.varFunctions[variable].tip);
							td.tooltip({"track": true});
						}
						
						/* set click handler to passed function */
						td.click(function() {
							/* call passed function with variable's value as argument */
							options.varFunctions[variable].func(finalVal);
						});
					}
				});

				/* if readonly attribute is set, add empty table td */
				if (value._attrReadonly) {
					$.create("td", {"colSpan": "2"}, "&nbsp;").appendTo(row);
				/* if readonly attribute is not set, add buttons for edit and delete */
				} else {
					/* create "button" for editing */
					var img = $.create("img", {"src": "ui/img/e.gif", "alt": "edit"});
					img.click(function(e) {
						addOrEditItem(key);
						scrollTo(0, 0);
					});

					/* change image when mouse is over it */
					img.hover(
						function() {
							$(this).attr("src", "ui/img/e2.gif");
						},
						function() {
							$(this).attr("src", "ui/img/e.gif");
						}
					);
					$.create("td", {}, img).appendTo(row);

					/* create "button" for deleting */
					img = $.create("img", {"src": "ui/img/minus.gif", "alt": "delete"});
					img.click(function(e) {
						/* if router is offline — return */
						if (isRouterOffline()) {
							return;
						}

						/* unhilight previous selected item */
						$(".selected", c.table).removeClass("selected");

						/* highlight selected item */
						$(this).parents("tr").addClass("selected");

						/* confirm for deleting */
						deleteConfirm(key);
					});

					/* change image when mouse is over it */
					img.hover(
						function() {
							$(this).attr("src", "ui/img/minus2.gif");
						},
						function() {
							$(this).attr("src", "ui/img/minus.gif");
						}
					);
					$.create("td", {}, img).appendTo(row);
				}
			});
			
			/* create scrollable div */
			var div = $.create("div", {"className": "scrollable"});
	
			/* add current table to scrollable div */
			c.table.wrap(div);
			
			/* rendering of table takes a long time, set timeout and call minmax() to fix IE */
			setTimeout(function() { $("div.scrollable", c.form).minmax(); }, 50);
			
			/* add div for showing delete confirm message */
			c.form.prepend($.create("div", {"className": "message error_message"}));
		};
	};
	
	/*
	 * Returns new object for creating list.
	 * 
	 * options — list options.
	 */
	this.createList = function(options) {
		return new List(this, options);
	};
}

/*
 * Adds a new item to the menu.
 * path — place for a new item (e.g., "Network:Interfaces" means menu Network, submenu Interfaces).
 * name — name of the menu item.
 * func — name of the function in a Controllers object to call when user clicks on the menu item.
 * params — function parameters.
 * 
 * Example of the menu structure is given below.
 * <ul class="treeview" id="menu">
 *		<li><span>System</span>
 *			<ul>
 *				<li><span><a href="#" onclick="Controllers.webface()">Interface</a></span></li>
 *				<li><span><a href="#" onclick="Controllers.general()">General</a></span></li>
 *			</ul>
 *		</li>
 *		<li><span>Network</span>
 *		    <ul>
 *		        <li><span>Interfaces</span>
 *					<ul>
 *						<li><span><a href="#" onclick="Controllers.iface('eth0')">eth0</a></span></li>
 *						<li><span><a href="#" onclick="Controllers.iface('eth1')">eth1</a></span></li>
 *					</ul>
 *				</li>
 *			</ul>
 *		</li>
 *	</ul>
 */
function addItem(path, name, func, params) {
	/* menu element */
	var idMenu = "#menu";
	
	/* context which is set when menu functions are called */
	var defaultContext = Controllers;
	
	var curLevel = idMenu;
	var pathElems = path.split(":");
	for (var pathElem in pathElems) {
		/* check if the corresponding submenu is exist */
		if ($(" > li > span:contains('" + _(pathElems[pathElem]) + "')", curLevel).length == 0) {
			/* if not, add it */
			$(curLevel).append("<li><span>" + _(pathElems[pathElem]) + "</span><ul></ul></li>");
		}
		/* change current level in the menu */
		curLevel = $(" > li > span:contains('" + _(pathElems[pathElem]) + "')", curLevel).next();
	}
	
	/* create link object */
	var link = $.create("a", {}, _(name)).click(function() {
        if (params) {
            defaultContext[func].apply(defaultContext, params);
        } else {
            defaultContext[func]();
        }

        /* highlight selected item */
        $("a", idMenu).removeClass("clicked");
        $(this).addClass("clicked");
    });
	
	/* create menu item and add it to the menu */
	$.create("li", {}, $.create("span", {}, link)).appendTo(curLevel);
}

/*
 * Update specified fields:
 * update our local KDB, and if value of field was changed — update field and alert user
 * by appending text to field name.
 * ! Field must have ID identical to it's name !
 * 
 * fields — name (or array with names) of field to update.
 * showAlertText — if field was updated — add message.
 */
function updateFields(fields, showAlertText) {
	var oldValues = new Object();
	
	/* convert single field to array */
	if (typeof fields == "string") {
		var field = fields;
		fields = new Array();
		fields.push(field);
	}
	
	/* save old values of fields */
	$.each(fields, function(num, field) {
		oldValues[field] = config.get(field);
	});
	
	/* update local KDB */
	config.updateValues(fields);
	
	/* check if fields was updated */
	$.each(fields, function(num, field) {
		if (oldValues[field] != config.get(field)) {
			/* set new value */
			$("#" + field).val(config.get(field));
			
			/* set class */
			$("#" + field).addClass("fieldUpdated");
			
			/* add info text to field name */
			if (showAlertText) {
				var widgetText = $("#" + field).parents("tr").children(".tdleft");
				var alertText = $.create("span", {"style": "color: red", "className": "alertText"}, " updated");
				widgetText.append(alertText);
			}
		}
	});
}
