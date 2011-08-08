/**
 * jQuery DOMEC (DOM Elements Creator) 0.3.1
 *
 * Copyright (c) 2008 Lukasz Rajchel (lukasz@rajchel.pl | http://lukasz.rajchel.pl)
 * Dual licensed under the MIT (http://www.opensource.org/licenses/mit-license.php) 
 * and GPL (http://www.opensource.org/licenses/gpl-license.php) licenses.
 *
 * Syntax:
 * $.create(element[, attributes[, children]])
 *
 * Parameters:
 * - string element - name of the element to create
 * - object|array attributes - element properties to be set
 * - string|array children - child elements (could also contain text value)
 *
 * Changelog:
 * 0.3.1 (2008.04.10)
 * - code optimization 
 *
 * 0.3 (2008.04.04)
 * - plugin function renamed from new to create (works now in IE)
 *
 * 0.2.1 (2008.04.01)
 * - namespace added
 * - fixed dates in changelog
 * - comments added
 *
 * 0.2 (2008.03.19)
 * - attributes and children parameters added
 * - changelog added
 *
 * 0.1 (2008.03.18)
 * - initial release
 */
 
(function($) {
 
 	// register jQuery extension
	$.extend({
		create: function(element, attributes, children) {
			var elem;
			
			// create new element
			// this piece of code was taken from FlyDOM plugin and slightly modified
		    if (jQuery.browser.msie && (element == 'input' || element == 'select')) {
		        // IE will not allow you to modify the type & name attribute after an element is
		        // created, so we must create the input element with the type attribute.
				var inputType = attributes.type ? " type='" + attributes.type + "' " : "";
				var inputName = attributes.name ? " name='" + attributes.name + "' " : "";
				var inputChecked = attributes.checked ? " checked='" + attributes.checked + "' " : "";
				
		        elem = $(document.createElement("<" + element + inputType + inputName 
		        	+ inputChecked + " />"));
		    } else {
		        // This is for every other element
		        elem = $(document.createElement(element));
		    }
			
			// add passed attributes
			if (typeof(attributes) == 'object') {
				for (key in attributes) {
					elem.attr(key, attributes[key]);
				}
			}
			
			// add passed child elements
			if (typeof(children) == 'string') {
				elem.html(children);
			} else if (typeof(children) == 'object') {
				for (i = 0; i < children.length; i++) {
					elem.append(children[i]);
				}
			}
			return elem;
		}
	});

})(jQuery);