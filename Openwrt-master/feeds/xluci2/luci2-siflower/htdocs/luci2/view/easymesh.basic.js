L.ui.view.extend({
	file_exec: L.rpc.declare({
        object: 'file',
        method: 'exec',
        params: [ 'command', 'params' ],
    }),

	timerWaitStatus:async function(interaval, status, flag) {
		var self = this;
        try {
            var detectionResult = await new Promise(function(resolve, reject) {
                var timerCount = 0;
                var intervalId = setInterval(async function() {
                    if(status == 'null') {
                        timerCount += 2;
                        if (timerCount >= interaval) {
                            console.log('Timer reached');
                            clearInterval(intervalId);
                            resolve(1);
                        }
                    } else {
                        var easymeshPromise = L.uci.callLoad('easymesh');
                        var wirelessPromise = L.uci.callLoad('wireless');
                        var promiseList = [easymeshPromise, wirelessPromise]
                        if(status == 'can_enable_conn') {
                            var check_conn_enable = self.file_exec('/bin/sf_easymesh', ["check_conn_enable"])
                            promiseList.push(check_conn_enable)
                        } else if(status == 'can_enable_wps') {
                            var check_wps_enable = self.file_exec('/bin/sf_easymesh', ["check_wps_enable"])
                            promiseList.push(check_wps_enable)
                        } else if(status == 'can_update_wifi_config') {
                            var check_can_update_wifi_config = self.file_exec('/bin/sf_easymesh', ["check_can_update_wifi_config"])
                            promiseList.push(check_can_update_wifi_config)
                        }
                        var PromiseResult = await Promise.all(promiseList);
						console.log(PromiseResult)
                        var result = PromiseResult[0]['status'][status]
                        if (result && result == flag) {
                            clearInterval(intervalId);
                            console.log('Detected ' + status);
                            resolve(1);
                        } else {
							timerCount += 2;
							if (timerCount >= interaval) {
								console.log('Timer reached');
								clearInterval(intervalId);
								resolve(0);
							}
						}
                    }
                }, 2000);
            });
            return detectionResult;
        } catch (error) {
            console.log(error)
            return null;
        }
    },

	updateConfig: async function() {
        var easymeshPromise = L.uci.callLoad('easymesh');
        var wirelessPromise = L.uci.callLoad('wireless');
        try {
            var PromiseResult = await Promise.all([easymeshPromise, wirelessPromise]);
            return PromiseResult;
        } catch (error) {
            console.error(error);
            return null;
        }
    },

	forbidOpt: function() {
		$('#enable_easymesh_switch').prop('disabled', true).addClass('disabled-element').css({'pointer-events': 'none'});
		$('#load').prop('disabled', true).addClass('disabled-element').css({'pointer-events': 'none'});;
		$('#wps').prop('disabled', true).addClass('disabled-element').css({'pointer-events': 'none'});;
	},

	allowOpt: function() {
		$('#enable_easymesh_switch').prop('disabled', false).removeClass('disabled-element').css({'pointer-events': ''});
		$('#load').prop('disabled', false).removeClass('disabled-element').css({'pointer-events': ''});
		this.updateCanWps();
	},

	updateCanSet: async function() {
        var PromiseResult = await this.updateConfig();
        var is_first_load = PromiseResult[0]['status'].is_first_load;
		console.log(is_first_load)
        if(is_first_load == 'false') {
            $('#backhaul_ssid').prop('disabled', true)
            $('#backhaul_key').prop('disabled', true)
            $('#device_role select').prop('disabled', true)
            $('#backhaul_form').prop('disabled', true)
        } else if(is_first_load == 'true') {
            $('#backhaul_ssid').prop('disabled', false)
            $('#backhaul_key').prop('disabled', false)
            $('#device_role select').prop('disabled', false)
            $('#backhaul_form').prop('disabled', false)
        }
    },

	updateCanWps: async function() {
        this.file_exec('/bin/sf_easymesh', ["check_wps_enable"])
        var PromiseResult = await this.updateConfig();
        var can_enable_wps = PromiseResult[0]['status'].can_enable_wps;
        if(can_enable_wps == 'false') {
            $('#wps').prop('disabled', true).addClass('disabled-element').css({'pointer-events': 'none'});;
        } else if(can_enable_wps == 'true') {
            $('#wps').prop('disabled', false).removeClass('disabled-element').css({'pointer-events': ''});
        }
		console.log("updateCanWps")
    },

	handleSetMode: async function () {
        var PromiseResult = await this.updateConfig();
        var enbale = PromiseResult[0]['status'].enable;
        var is_set_mode = PromiseResult[0]['status'].is_set_mode;
        if(enbale == '0') {
            // ui.addNotification(null, E('p', [ _('Easymesh not enable') ]));
			console.log('Easymesh not enable')
            return -1;
        } else if(is_set_mode == 'true') {
            return;
        } else {
            var role = $('#device_role select').val();
            this.file_exec('/bin/sf_easymesh', ["set_management_mode", role])
        }
    },

	bytelen: function(x) {
        return new Blob([x]).size;
    },

    checkInput: async function() {
        var fhslen = this.bytelen($('#fssid_input').val());
        var fhklen = this.bytelen($('#fkey_input').val());
        var bhblen = this.bytelen($('#bssid_input').val());
        var bhklen = this.bytelen($('#bkey_input').val());
        if(fhslen > 32 || bhblen > 32) {
            // ui.addNotification(null, E('p', [ _('Ssid is more than 32 chars') ]));
			console.log('Ssid is more than 32 chars')
            return -1
        }
        if(fhklen < 8 || fhklen > 63 || bhklen < 8 || bhklen > 63) {
            // ui.addNotification(null, E('p', [ _('Key must between 8 and 63 chars') ]));
			console.log('Key must between 8 and 63 chars')
            return -1
        }
        return 0;
    },

	handleSetControllerConfig: async function () {
        var PromiseResult = await this.updateConfig();
        var role = $('#device_role select').val();

        if(PromiseResult[0]['status'].enable == '0') {
            return;
        }

        var role = $('#device_role select').val();
        if(role == "controller") {
            var fhs = $('#fssid_input').val();
            var fhk = $('#fkey_input').val();
            var bhb = $('#bssid_input').val();
            var bhk = $('#bkey_input').val();
            var encr = $('#encryption_select').val();
            var is_first_load = PromiseResult[0]['status'].is_first_load;
            if(is_first_load == 'true') {
                this.file_exec('/bin/sf_easymesh', ["set_controller_config", fhs, fhk, encr, bhb, bhk]);
                this.file_exec('/bin/sf_easymesh', ["load_for_web"]);
            } else {
                var can_update_wifi_config = await this.timerWaitStatus(20, 'can_update_wifi_config', 'true');
                if(can_update_wifi_config) {
                    this.file_exec('/bin/sf_easymesh', ["set_wifi_config", fhs, fhk, encr])
                    this.file_exec('/bin/sf_easymesh', ["update_wifi_config"])
                }
            }
        } else {
            var form = $('#bform_select').val();
            this.file_exec('/bin/sf_easymesh', ["set_backhaul_form", form]);
            this.file_exec('/bin/sf_easymesh', ["load_for_web"])
        }
    },

	execute: async function() {
		var self = this;
		$('input,select').addClass('form-control').css('width', '55%');
		$('#settings div.row').css('margin', '10px 0');
		$('#device_role select').change(function () {
			const selectedValue = $('#device_role select').val();
			if (selectedValue === 'controller') {
				$('#backhaul_form').hide();
				$('#backhaul_ssid').show();
				$('#backhaul_key').show();
			} else {
				$('#backhaul_form').show();
				$('#backhaul_ssid').hide();
				$('#backhaul_key').hide();
			}
		});

		$('#load').click(async function () {
			self.forbidOpt()
			if(await self.handleSetMode() == -1){
				self.allowOpt();
				return;
			}

			if(await self.checkInput() == -1){
				self.allowOpt();
				return;
			}

			var is_set_mode = await self.timerWaitStatus(20, 'is_set_mode', 'true');
			if(!is_set_mode) {
				// ui.addNotification(null, E('p', [ _('Set mode fail') ]));
				console.log('Set mode fail')
			} else {
				await self.handleSetControllerConfig();
			}
			var can_enable_conn = await self.timerWaitStatus(30, 'can_enable_conn', 'true');
			if(!can_enable_conn) {
				// ui.addNotification(null, E('p', [ _('Easymesh driver load fail') ]));
				console.log('Easymesh driver load fail')
			}
			self.updateCanSet();
			self.allowOpt()
		})
		$('#wps').click(async function () {
			self.forbidOpt();
			var PromiseResult = await self.updateConfig();
			var enbale = PromiseResult[0]['status'].enable;
			if(enbale == '0') {
				// ui.addNotification(null, E('p', [ _('Easymesh not enable') ]));
				console.log('Easymesh not enable')
				self.allowOpt();
				return;
			}

			var can_enable_conn = PromiseResult[0]['status'].can_enable_conn;
			if(can_enable_conn == 'false') {
				// ui.addNotification(null, E('p', [ _('Config not load') ]));
				console.log('Config not load')
				self.allowOpt();
				return;
			}

			var can_enable_wps = await self.timerWaitStatus(20, 'can_enable_wps', 'true');
			if(!can_enable_wps) {
				// ui.addNotification(null, E('p', [ _('Can not wps') ]));
				console.log('Can not wps')
				return;
			} else {
				self.file_exec('/bin/sf_easymesh', ["enable_wps"])
			}
			// ui.addNotification(null, E('p', [ _('WPSing, please wait 2 minutes') ]));
			console.log('WPSing, please wait 2 minutes')
			await self.timerWaitStatus(120, 'null', 'null');
			self.allowOpt();

		})

		$('#enable_easymesh_switch').click(async function () {
			self.forbidOpt()
			var status = $('#enable_easymesh_switch').prop('status');
			var opt = status == 1 ? "enable" : "disable";
			self.file_exec('/bin/sf_easymesh', [opt])
			if (opt == 'enable') {
				var result = await self.timerWaitStatus(20, 'enable', '1');
			} else {
				var result = await self.timerWaitStatus(20, 'enable', '0');
			}
			if(!result) {
				console.log(opt + ' fail, please retry')
				self.allowOpt();
			} else {
				var PromiseResult = await self.updateConfig();
				var easymesh = PromiseResult[0]
				var wireless = PromiseResult[1]['default_radio0']
				console.log(easymesh)
				console.log(wireless)
				console.log(wireless['ssid'])
				$('#fssid_input').val(wireless['ssid'])
				$('#fkey_input').val(wireless['key'])
				$('#encryption_select').val(wireless['encryption']);
				$('#bssid_input').val(wireless['multi_ap_backhaul_ssid']);
				$('#bkey_input').val(wireless['multi_ap_backhaul_key']);
				$('#device_role select').val(easymesh['status']['management_mode']);

				if (opt == "enable") {
					$('#enable_easymesh_switch').addClass('switch_on').removeClass('switch_off');
					$('#enable_easymesh_switch').prop('status', '0');
					$('#fronthaul_ssid').show();
					$('#fronthaul_key').show();
					$('#encryption').show();
					$('#device_role').show();
					$('#backhaul_form').show();
					$('#backhaul_ssid').show();
					$('#backhaul_key').show();
					$('#device_role select').trigger('change');
				} else {
					$('#enable_easymesh_switch').addClass('switch_off').removeClass('switch_on');
					$('#enable_easymesh_switch').prop('status', '1');
					$('#fronthaul_ssid').hide();
					$('#fronthaul_key').hide();
					$('#encryption').hide();
					$('#device_role').hide();
					$('#backhaul_form').hide();
					$('#backhaul_ssid').hide();
					$('#backhaul_key').hide();
				}
				self.updateCanSet();
				self.allowOpt();
			}
		});
		var PromiseResult = await self.updateConfig();
		var easymesh = PromiseResult[0]
		var wireless = PromiseResult[1]['default_radio0']
		var enable = easymesh['status'].enable;
		$('#fssid_input').val(wireless['ssid'])
		$('#fkey_input').val(wireless['key'])
		$('#encryption_select').val(wireless['encryption']);
		$('#bssid_input').val(wireless['multi_ap_backhaul_ssid']);
		$('#bkey_input').val(wireless['multi_ap_backhaul_key']);
		$('#device_role select').val(easymesh['status']['management_mode']);
		if (enable === '1') {
            $('#enable_easymesh_switch').addClass('switch_on').removeClass('switch_off');
            $('#enable_easymesh_switch').prop('status', '0');
            $('#fronthaul_ssid').show();
            $('#fronthaul_key').show();
            $('#encryption').show();
            $('#device_role').show();
            $('#backhaul_form').show();
            $('#backhaul_ssid').show();
            $('#backhaul_key').show();
			$('#device_role select').trigger('change');
        } else {
            $('#enable_easymesh_switch').addClass('switch_off').removeClass('switch_on');
            $('#enable_easymesh_switch').prop('status', '1');
            $('#fronthaul_ssid').hide();
            $('#fronthaul_key').hide();
            $('#encryption').hide();
            $('#device_role').hide();
            $('#backhaul_form').hide();
            $('#backhaul_ssid').hide();
            $('#backhaul_key').hide();
        }
		self.updateCanSet();
		self.updateCanWps();
	}
});