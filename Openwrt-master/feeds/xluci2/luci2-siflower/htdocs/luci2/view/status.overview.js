L.ui.view.extend({

    execute: function() {

		function make_memory_info(v, total){
			return ('%d kB / %d kB (%d%%)').format(v / 1024, total / 1024, Math.round(v / total * 100));
		}

		function formatUptime(seconds) {
			var days = Math.floor(seconds / (24 * 3600));
			var hours = Math.floor((seconds % (24 * 3600)) / 3600);
			var minutes = Math.floor((seconds % 3600) / 60);

			return days + " " + L.tr(' days') + " " + hours + " " + L.tr(' hours') + " " + minutes + " " + L.tr(' mins');
		}

		function renderInfo(data, info) {
			let html = `
				<div class="title_line">
					<h3 class="title_line" style="border-bottom:solid 1px #c9c9c9;">${L.tr(data.label)}</h3>
					<div style="margin-top:20px">
			`;

			$.each(data.items, function(i, v) {
				let htmls = `
					<div class="row">
						<div>${L.tr(v.label)}</div>
						<div class="form-control-static">
							${info.hasOwnProperty(v.value)
								? (v.value === "uptime" ? formatUptime(info[v.value]) : info[v.value])
								: ''
							}
						</div>
					</div>
				`;
				html += htmls
			});

			html += `</div></div>`;
			return html;
		}

		function showInfoList(data, info) {
			$("#infoList").empty();
			let html ="";

			$.each(data, function(i, v) {
				if (v.items) {
					html += renderInfo(v, info);
				}
			});

			$("#infoList").append(html);
		}

		L.system.getInfo().then(function(info) {
			$.ajax(L.globals.resource + `/vendor/${L.globals.vendorName}/resources/overView.json`,{
				cache:    true,
				dataType: 'text',
				dataType: 'json',
				success:  function(data) {
					showInfoList(data, info);
				}
			});

			var total = info.memory.total;
			$('#memory_1').attr("style", "width: " + Math.round((info.memory.free + info.memory.buffered) / total * 100)+"%");
			$('#memory_1_text').text(make_memory_info(info.memory.free + info.memory.buffered, total));
			$('#memory_2').attr("style", "width: " + Math.round(info.memory.free / total * 100)+"%");
			$('#memory_2_text').text(make_memory_info(info.memory.free, total));
			$('#memory_3').attr("style", "width: " + Math.round(info.memory.shared / total * 100)+"%");
			$('#memory_3_text').text(make_memory_info(info.memory.shared, total));
			$('#memory_4').attr("style", "width: " + Math.round(info.memory.buffered / total * 100)+"%");
			$('#memory_4_text').text(make_memory_info(info.memory.buffered, total));
		});
    }
});
