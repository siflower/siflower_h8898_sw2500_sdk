L.ui.view.extend({
	execute: function() {
		$('#btn_logout').click(function() {
			var form = $('<p />').text(L.tr('Really logout?'));
			L.ui.dialog(L.tr('Logout'), form, {
				style: 'confirm',
				confirm: function() {
					L.session.destroy('ubus', 'session', 'destroy');
					location.replace('/');
				},
			});
		});
	}
});
