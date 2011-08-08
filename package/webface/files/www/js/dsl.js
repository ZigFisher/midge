rate_list8 = new Array(192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840);
rate_list16 = new Array(192,256,320,384,512,640,768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840);
rate_list32_v1 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696);
rate_list32_v2 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10176);
rate_list64 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12736);
rate_list128 = new Array(768,1024,1280,1536,1792,2048,2304,2560,3072,3584,3840,4096,4608,5120,5696,6144,7168,8192,9216,10240,11520,12800,14080);


function freeList(l){
    
    while ( l.length > 0 ){
		//	 alert("remove " + l.options[l.length-1].value);
        try{
            l.options.remove(l.length-1);
        }catch(ex){
			//	     alert("Cannot remove elem l.length-1");
            l.options[l.length-1] = null;
    	    l.length--;
		};
    };
};
	

function rate_list(l,st,end,step,cur){
    freeList(l);
	ind=0;
	ind_res=0;
	
    for(i=st;i<=end;i+=step){
    	el = new Option;
    	el.value = i;
    	el.text = i;
		el.selected = 0;
		if( i == cur ){
    	    el.selected = 1;
			ind_res = ind;
		};
		try{
			l.options.add(el);
		} catch(ex){
			l.options.add(el,null);
		};
		ind++;
    };
	l.selectedIndex = ind_res;
};

function fixed_rate_list(l,cur,rates){
    freeList(l);
	ind=0;
	ind_res=0;
	val_res=0;

    for(i=0;i<rates.length;i++){
    	el = new Option;
    	el.value = rates[i];
    	el.text = rates[i];
		el.selected = 0;
		
		if( cur>0 && Math.abs(rates[i]-cur) < Math.abs(val_res-cur) ){
			ind_res = i;
			val_res = rates[i];
		};
		try{
			l.options.add(el);
		} catch(ex){
			l.options.add(el,null);
		};
    };

	// Other speed selection	
   	el = new Option;
   	el.value = -1;
   	el.text = 'other';
	if( cur < 0 ){
		el.selected = 0;
		ind_res = i;
	}
	l.options.add(el);

	// Select current rate
	l.selectedIndex = ind_res;
	l[ind_res].selected = 1;
};


function OnChangeSG16Code(){

	//alert("Start SG16 change");
    TCPAM = new Array();
	TCPAM[0] = new Array();
	TCPAM[0][0] = "tcpam4";
	TCPAM[0][1] = "TCPAM4";		
	TCPAM[1] = new Array();	
	TCPAM[1][0] = "tcpam8";
	TCPAM[1][1] = "TCPAM8";		
	TCPAM[2] = new Array();	
	TCPAM[2][0] = "tcpam16";
	TCPAM[2][1] = "TCPAM16";		
	TCPAM[3] = new Array();	
	TCPAM[3][0] = "tcpam32";
	TCPAM[3][1] = "TCPAM32";		

    preact = $('cfg').options[$('cfg').selectedIndex].value;
    annex = $('annex').options[$('annex').selectedIndex].value;
    mode = $('mode').options[$('mode').selectedIndex].value;
    tcpam = $('code').options[$('code').selectedIndex].value;

	ind = $('rate').selectedIndex;
	if( ind < 0 || ind==undefined ){
		ind = 0;
		rate = 64
	}else{
		rate = $('rate').options[ind].value;
	}


    if( preact == "preact" && annex == "F"){
		// TCPAM 16/32
		// Rate: 192 - 5696 for master, automatic for slave
		freeList($('code'));
		if( mode == "slave" ){
			var el = new Option;
			el.value = TCPAM[3][0];
			el.text = TCPAM[3][1];
			$('code').options.add(el);
			$('code').disabled = 1;
			freeList($('rate'));
			$('rate').options[0] = new Option("automatic");
			$('rate').disabled = 1;
		} else {
			$('code').disabled = 0;
			for(i=3;i>=2;i--){
				var el = new Option;
				el.value = TCPAM[i][0];
				el.text = TCPAM[i][1];
				if( TCPAM[i][0] == tcpam ){
					el.selected = 1;
				};
				$('code').options.add(el);
			};
			$('rate').disabled = 0;
			tcpam = $('code').options[$('code').selectedIndex].value;
			if( tcpam == "tcpam16" )
				rate_list($('rate'),192,2304,64,rate);
			else
				rate_list($('rate'),192,5696,64,rate);
		};
    }else if( preact == "preact"){
		freeList($('code'));
		var el = new Option;
		el.value = TCPAM[2][0];
		el.text = TCPAM[2][1];
		$('code').options.add(el);
		$('code').disabled = 1;

		if( mode == "slave" ){
			freeList($('rate'));
			$('rate').options[0] = new Option("automatic");
			$('rate').disabled = 1;
		} else {
			$('rate').disabled = 0;
			rate_list($('rate'),192,2304,64,rate);	
		};
    } else {
		freeList($('code'));
		$('code').disabled = 0;
		for(i=3;i>=0;i--){
			var el = new Option;
			el.value = TCPAM[i][0];
			el.text = TCPAM[i][1];
			if( TCPAM[i][0] == tcpam ){
				el.selected = 1;
			};
			$('code').options.add(el);
			if( TCPAM[i][0] == tcpam ){
				$('code').selectedIndex = 3-i;
			};
		};

		$('rate').disabled = 0;
		tcpam = $('code').options[$('code').selectedIndex].value;
		if( tcpam == "tcpam4" ){
			rate_list($('rate'),64,704,64,rate);
		} else if( tcpam == "tcpam8" ){
			rate_list($('rate'),192,1216,64,rate);
		} else  if( tcpam == "tcpam16" ) {
			rate_list($('rate'),192,3840,64,rate);
		} else  if( tcpam == "tcpam32" ){
			rate_list($('rate'),256,6016,64,rate);
		};
    };
};



function OnChangeSG17Code()
{
    TCPAM = new Array();
	TCPAM[0] = new Array();
	TCPAM[0][0] = "tcpam8";
	TCPAM[0][1] = "TCPAM8";		
	TCPAM[1] = new Array();
	TCPAM[1][0] = "tcpam16";
	TCPAM[1][1] = "TCPAM16";		
	TCPAM[2] = new Array();	
	TCPAM[2][0] = "tcpam32";
	TCPAM[2][1] = "TCPAM32";		
	TCPAM[3] = new Array();	
	TCPAM[3][0] = "tcpam64";
	TCPAM[3][1] = "TCPAM64";		
	TCPAM[4] = new Array();	
	TCPAM[4][0] = "tcpam128";
	TCPAM[4][1] = "TCPAM128";		

    mode = $('mode').options[$('mode').selectedIndex].value;
    tcpam = $('code').options[$('code').selectedIndex].value;
	clkmode_ind = $('clkmode').selectedIndex;
	annex_ind = $('annex').selectedIndex;
	ind = $('rate').selectedIndex;
	pbomode = $('pbomode').checked;
	var pboval = document.getElementById('pboval');
	var mre = document.getElementById('mrate');
	//	alert('ratetype=' + $('ratetype').checked );
	if( ind < 0 ){
		ind = 0;
		rate = 192;
	}else{
		rate = $('rate').options[ind].value;
	}

    if( mode == "slave" ){
	    freeList($('rate'));
	    $('rate').options[0] = new Option("automatic");
	    $('rate').disabled = 1;
	    freeList($('code'));
	    $('code').options[0] = new Option("automatic");
	    $('code').disabled = 1;
	    freeList($('clkmode'));
	    $('clkmode').options[0] = new Option("automatic");
	    $('clkmode').disabled = 1;
	    freeList($('annex'));
	    $('annex').options[0] = new Option("automatic");
	    $('annex').disabled = 1;
	    $('pbomode').disabled = 1;
	    var base = document.getElementById('rate_td');
	    if( mre != null )
		    base.removeChild(mre);
	    base = document.getElementById('pbomode_td');
	    if( pboval != null )
		    base.removeChild(pboval);
    } else {
	    freeList($('code'));
	    $('code').disabled = 0;
	    if( $('chipver').value == 'v1' ){
		    start=1;
		    end=2;
	    }else{
		    start=0;
		    end=4;
	    }
	    flag = 0;
	    for(i=end;i>=start;i--){
    	    var el = new Option;
	        el.value = TCPAM[i][0];
	        el.text = TCPAM[i][1];
    	    if( TCPAM[i][0] == tcpam ){
		    	el.selected = 1;
		    };
		    $('code').options.add(el);
		    if( TCPAM[i][0] == tcpam ){
		        $('code').selectedIndex = end-i;
		        flag = 1;
		    }
	    };
	    if( !flag ){
	        $('code').selectedIndex = 0;
	    }
		$('rate').disabled = 0;
		tcpam = $('code').options[$('code').selectedIndex].value;
		if( $('chipver').value == 'v1' ){
		    if( tcpam == "tcpam16" ) {
				fixed_rate_list($('rate'),rate,rate_list16);
			} else if( tcpam == "tcpam32" ){
				fixed_rate_list($('rate'),rate,rate_list32_v1);
			} else if( tcpam == "tcpam8" ){
				fixed_rate_list($('rate'),rate,rate_list8);
			};
		}else{
			if( tcpam == "tcpam128" ) {
				fixed_rate_list($('rate'),rate,rate_list128);
			} else if( tcpam == "tcpam64" ){
				fixed_rate_list($('rate'),rate,rate_list64);
			} else if( tcpam == "tcpam32" ){
				fixed_rate_list($('rate'),rate,rate_list32_v2);
			} else if( tcpam == "tcpam16" ){
				fixed_rate_list($('rate'),rate,rate_list16);
			} else if( tcpam == "tcpam8" ){
				fixed_rate_list($('rate'),rate,rate_list8);
			};
		}
		
		if( rate < 0 && mre == null ){
			var base = document.getElementById('rate_td');
			var oe = document.getElementById('rate');
			var ne = document.createElement('input');
			ne.setAttribute('type','text');
			ne.setAttribute('id','mrate');
			ne.setAttribute('class','edit');
			ne_name='sys_pcicfg_s'+$('pcislot').value+'_'+$('pcidev').value+'_mrate';
			ne.setAttribute('name',ne_name);
			ne.setAttribute('value',$('hmrate').value);
			base.insertBefore(ne,oe.nextSibling);
		}else if( rate > 0 && mre != null ){
			var base = document.getElementById('rate_td');
			base.removeChild(mre);
		}
		// Clock mode		
		freeList($('clkmode'));
		$('clkmode').disabled = 0;
		$('clkmode').options[0] = new Option("plesio");
		$('clkmode').options[1] = new Option("plesio-ref");
		$('clkmode').options[2] = new Option("sync");
		$('clkmode').selectedIndex=clkmode_ind;

		// PBO
		$('pbomode').disabled = 0;
		if( pbomode == true && pboval == null ){
			var base = document.getElementById('pbomode_td');
			var oe = document.getElementById('pbomode');
			var ne = document.createElement('input');
			ne.setAttribute('type','text');
			ne.setAttribute('id','pboval');
			ne.setAttribute('class','edit');
			ne.setAttribute('size','24');
			ne.setAttribute('maxsize','24');
			ne_name='sys_pcicfg_s'+$('pcislot').value+'_'+$('pcidev').value+'_pboval';
			ne.setAttribute('name',ne_name);
			ne.setAttribute('value',$('hpboval').value);
			base.insertBefore(ne,oe.nextSibling);
		}else if( pbomode == false && pboval != null ){
			var base = document.getElementById('pbomode_td');
			base.removeChild(pboval);
		}
		
		// Annex
		freeList($('annex'));
		$('annex').disabled = 0;
		$('annex').options[0] = new Option("Annex A");
        $('annex').options[0].value = 'A';
		$('annex').options[1] = new Option("Annex B");
        $('annex').options[1].value = 'B';
		$('annex').selectedIndex=annex_ind;
    }
};

function eocProfiles(){

	for(var i=1;;i++){
		s = $('rate' + i);
		//alert('s = ' + s);
		if( s == null ){
			break;
		}
		ind = s.selectedIndex;
		//alert('ind = ' + ind);
		if( ind == undefined ){
			break;
		}

		if( ind < 0 ){
			ind = 0;
			rate = 192;
		}else{
			rate = s.options[ind].value;
		}
		
		tc = $('tcpam' + i);
	    tcpam = $('tcpam' + i).options[$('tcpam' + i).selectedIndex].value;
		if( tcpam == "tcpam128" ) {
			fixed_rate_list(s,rate,rate_list128);
		} else if( tcpam == "tcpam64" ){
			fixed_rate_list(s,rate,rate_list64);
		} else if( tcpam == "tcpam32" ){
			fixed_rate_list(s,rate,rate_list32_v2);
		} else if( tcpam == "tcpam16" ){
			fixed_rate_list(s,rate,rate_list16);
		} else if( tcpam == "tcpam8" ){
			fixed_rate_list(s,rate,rate_list8);
		};

		var mre = document.getElementById('mrate' + i);
		if( rate < 0 ){
			mre.style.display="";
		}else if( rate > 0 ){
			mre.style.display='none';
		}
	}

	s = $('rate');
	//alert('s = ' + s);
	if( s == null ){
		return;
	}
	ind = s.selectedIndex;
	//alert('ind = ' + ind);
	if( ind == undefined ){
		return;
	}
	if( ind < 0 ){
		ind = 0;
		rate = 192;
	}else{
		rate = s.options[ind].value;
	}
	tc = $('tcpam');
	tcpam = $('tcpam').options[$('tcpam').selectedIndex].value;
	if( tcpam == "tcpam128" ) {
		fixed_rate_list(s,rate,rate_list128);
	} else if( tcpam == "tcpam64" ){
		fixed_rate_list(s,rate,rate_list64);
	} else if( tcpam == "tcpam32" ){
		fixed_rate_list(s,rate,rate_list32_v2);
	} else if( tcpam == "tcpam16" ){
		fixed_rate_list(s,rate,rate_list16);
	} else if( tcpam == "tcpam8" ){
		fixed_rate_list(s,rate,rate_list8);
	};
};


function eocIfSettings(){

    mode = $('mode').options[$('mode').selectedIndex].value;
	pbomode = $('pbomode').checked;
	var pboval = document.getElementById('pboval');

    if( mode == "slave" ){
		$('pbomode').disabled = 1;
		base = document.getElementById('pbomode_td');
		if( pboval != null )
			base.removeChild(pboval);
    } else {

		// PBO
		$('pbomode').disabled = 0;
		if( pbomode == true && pboval == null ){
			var base = document.getElementById('pbomode_td');
			var oe = document.getElementById('pbomode');
			var ne = document.createElement('input');
			ne.setAttribute('type','text');
			ne.setAttribute('id','pboval');
			ne.setAttribute('class','edit');
			ne.setAttribute('size','24');
			ne.setAttribute('maxsize','24');
			ne.setAttribute('name','pboval');
			ne.setAttribute('value',$('hpboval').value);
			base.insertBefore(ne,oe.nextSibling);
		}else if( pbomode == false && pboval != null ){
			var base = document.getElementById('pbomode_td');
			base.removeChild(pboval);
		}
    }

};


function aaa(){
    alert("aaa func");
}
