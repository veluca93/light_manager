var devices = {};
var unknown_devices = {};

function delete_device(id) {
    if (!confirm("Are you sure you want to delete device " + id + "?")) return;
    $.ajax({
        type: "DELETE",
        url: "/api/config/" + id
    }).done(function(_) {
        load_config();
    }).fail(function(data) {
        alert(data.responseJSON["msg"]);
    });
}

function pair_enabled(val, arr) {
    for (var i=0; i<arr.length; i++)
        if (val[0] == arr[i][0] && val[1] == arr[i][1])
            return true;
    return false;
}

function get_switch_modal(device_id, switch_id, sw) {
    var ret = "";
    var id = device_id + 'n' + switch_id;
    if (switch_id == 'ew') {
        switch_id = -1;
        for (var i in devices[device_id].switches)
            if (switch_id < i)
                switch_id = i;
        switch_id += 1;
    }
    var name = sw.name === undefined ? '' : sw.name;
    ret += '<div id="switch' + id + '" class="modal modal-fixed-footer grey darken-3">';
    ret += '  <form class="switchform" autocomplete="off">';
    ret += '  <input type="hidden" id="id' + id + '" name="device_id" value="' + device_id + '" />';
    ret += '  <input type="hidden" id="swid' + id + '" name="switch_id" value="' + switch_id + '" />';
    ret += '    <div class="modal-content">';
    if (name == '') {
        ret += '  <h4>Add switch to device ' + devices[device_id].name + '</h4>';
    } else {
        ret += '  <h4>Edit switch ' + name + ' of device ' + devices[device_id].name + '</h4>';
    }

    ret += '      <div class="row">';
    ret += '        <div class="input-field col s12">';
    ret += '          <input type="text" required="required" id="name' + id + '" name="name" value="' + name + '" />';
    ret += '          <label for="name' + id + '">Name</label>';
    ret += '        </div>';
    ret += '      </div>';
    ret += '      <div class="row">';
    ret += '        <div class="input-field col s12">';
    ret += '          <select multiple="multiple" name="buttons">';
    ret += '            <option value="" disabled="disabled">Choose buttons</option>';

    for (var i in devices) {
        for (var num = 0; num < devices[i].num_buttons; num++) {
            var selected = pair_enabled([parseInt(i), num], sw.buttons) ? 'selected="selected"' : '';
            ret += '    <option value="[' + i + ', ' + num + ']" ' + selected + '>' + devices[i].name + ' - ' + num + '</option>';
        }
    }

    ret += '          </select>';
    ret += '          <label>Enabled buttons</label>';
    ret += '        </div>';
    ret += '      </div>';

    ret += '      <div class="row">';
    ret += '        <div class="input-field col s12">';
    ret += '          <select multiple="multiple" name="pirs">';
    ret += '            <option value="" disabled="disabled">Choose PIRs</option>';

    for (var i in devices) {
        for (var num = 0; num < devices[i].num_pirs; num++) {
            var selected = pair_enabled([parseInt(i), num], sw.pirs) ? 'selected="selected"' : '';
            ret += '    <option value="[' + i + ', ' + num + ']" ' + selected + '>' + devices[i].name + ' - ' + num + '</option>';
        }
    }

    ret += '          </select>';
    ret += '          <label>Enabled PIRs</label>';
    ret += '        </div>';
    ret += '      </div>';

    ret += '      <div class="row">';
    ret += '        <div class="input-field col s12">';
    ret += '          <input type="number" min="0" id="pirtime' + id + '" name="pir_time" value="' + sw.pir_time + '" />';
    ret += '          <label for="pirtime' + id + '">Number of seconds the light stays on because of a PIR</label>';
    ret += '        </div>';
    ret += '      </div>';
    ret += '    </div>';

    ret += '    <div class="modal-footer grey darken-2">';
    ret += '      <button class="modal-action modal-close waves-effect waves-green btn">Ok</a>';
    ret += '      <button type="button" class="modal-action modal-close waves-effect waves-green btn btn-flat">Close</a>';
    ret += '    </div>';
    ret += '  </form>';
    ret += '</div>';
    return ret;
}

function gen_device_config(device, id) {
    var ret = "";
    ret += '<form class="s12 configform" id="form' + id + '" autocomplete="off">';
    if (id != 'new') {
        ret += '  <input type="hidden" id="id' + id + '" name="id" value="' + id + '" />';
    } else {
        ret += '    <div class="input-field col s12">';
        ret += '      <select name="id" id="newdevid">';
        ret += '      </select>';
        ret += '      <label>Id</label>';
        ret += '    </div>';
    }
    ret += '  <div class="row">';
    ret += '    <div class="input-field col s6">';
    ret += '      <input type="text" required="required" id="name' + id + '" name="name" value="' + (device.name === undefined ? '' : device.name) + '" />';
    ret += '      <label for="name' + id + '">Name</label>';
    ret += '    </div>';
    ret += '    <div class="input-field col s6 chkbox">';
    ret += '      <input type="checkbox" class="filled-in" name="battery_powered" id="batterypowered' + id + '" ' + (device.battery_powered ? 'checked="checked"' : '') + ' />';
    ret += '      <label for="batterypowered' + id + '">Battery powered</label>';
    ret += '    </div>';
    ret += '  </div>';

    ret += '  <div class="row">';
    ret += '    <div class="input-field col s6">';
    ret += '      <input type="number" required="required" min="0" id="numbuttons' + id + '" name="num_buttons" value="' + device.num_buttons + '" />';
    ret += '      <label for="numbuttons' + id + '">Number of buttons</label>';
    ret += '    </div>';
    ret += '    <div class="input-field col s6">';
    ret += '      <input type="number" required="required" min="0" id="numpirs' + id + '" name="num_pirs" value="' + device.num_pirs + '" />';
    ret += '      <label for="numpirs' + id + '">Number of PIRs</label>';
    ret += '    </div>';
    ret += '  </div>';

    ret += '  <div class="row center">';
    ret += '    <div class="input-field col s6">';
    ret += '      <button type="button" class="waves-effect waves-light btn red" onclick="delete_device(' + id + ')" >Delete</button>';
    ret += '    </div>';
    ret += '    <div class="input-field col s6">';
    ret += '      <button type="submit" class="waves-effect waves-light btn">' + (id == 'new' ? 'Add' : 'Edit') + ' <i class="material-icons right">send</i></button>';
    ret += '    </div>';
    ret += '  </div>';

    ret += '</form>';

    if (id != 'new') {
        ret += '<div class="row">';
        ret += '  <h5>Switches</h5>';
        ret += '  <div class="collection">';
        for (var i in device.switches) {
            ret += '<a class="collection-item modal-trigger grey darken-2 teal-text text-lighten-2" href="#switch' + id + 'n' + i + '">';
            ret += '  <i class="material-icons">edit</i> ' + device.switches[i].name;
            ret += '</a>';
        }
        ret += '    <a class="collection-item modal-trigger grey darken-2 teal-text text-lighten-2" href="#switch' + id + 'new">';
        ret += '      <i class="material-icons">add</i> Add';
        ret += '    </a>';
        ret += '  </div>';
        ret += '</div>';
        for (var i in device.switches) {
            ret += get_switch_modal(id, i, device.switches[i]);
        }
        ret += get_switch_modal(id, 'ew', {buttons: [], pirs: [], pir_time: 0});
    }
    return ret;
}


function gen_config_page() {
    var ret = '<ul class="collapsible" data-collapsible="expandable">';
    for (var i in devices) {
        ret += ' <li>';
        ret += '   <div class="collapsible-header grey darken-2"><i class="material-icons">edit</i>' + devices[i].name + "</div>";
        ret += '   <div class="collapsible-body">';
        ret += gen_device_config(devices[i], i);
        ret += '   </div>';
        ret += ' </li>';
    }
    ret += '     <li id="adddevice" class="hidden">';
    ret += '       <div class="collapsible-header grey darken-2"><i class="material-icons">add</i>Add</div>';
    ret += '       <div class="collapsible-body">';
    ret += gen_device_config({num_buttons: 0, num_pirs: 0}, 'new');
    ret += '       </div>';
    ret += '     </li>';
    ret += '   </ul>';
    return ret;
}

function start_loading() {
    $("#loader").removeClass("hidden");
    $("#content").addClass("blur");
    $("nav").addClass("blur");
}

function stop_loading() {
    $("#loader").addClass("hidden");
    $("#content").removeClass("blur");
    $("nav").removeClass("blur");
}

var poll_timeout;
var skip = false;

function turnoff() {
    if ($(".swlever").prop('disabled')) return false;
    if ($(".turnoff button").prop('disabled')) return false;
    skip = true;
    clearTimeout(poll_timeout);
    $(".swlever").prop('disabled', 'disabled');
    $(".turnoff button").prop('disabled', 'disabled');
    var cnt = 0;
    for (var dev in devices) {
        for (var sw in devices[dev].switches) {
            cnt += 1;
        }
    }
    var done = 0;
    for (var dev in devices) {
        for (var sw in devices[dev].switches) {
            var url = "/api/turn_off/" + dev + "/" + sw;
            $.ajax({
                type: "POST",
                url: url,
            }).done(function(data) {
                done += 1;
                if (done == cnt) {
                    skip = false;
                    poll_timeout = setTimeout(poll_events, 1000);
                }
            }).fail(function(data) {
                alert(data.responseJSON["msg"]);
                done += 1;
                if (done == cnt) {
                    skip = false;
                    poll_timeout = setTimeout(poll_events, 100);
                }
            });
        }
    }
    return true;
}

function toggle_switch(dev, sw) {
    if ($(".swlever").prop('disabled')) return false;
    if ($(".turnoff button").prop('disabled')) return false;
    skip = true;
    clearTimeout(poll_timeout);
    $(".swlever").prop('disabled', 'disabled');
    $(".turnoff button").prop('disabled', 'disabled');
    var url;
    if (devices[dev].switches[sw].status == "on") {
        url = "/api/turn_off/" + dev + "/" + sw;
    } else {
        url = "/api/turn_on/" + dev + "/" + sw;
    }
    $.ajax({
        type: "POST",
        url: url,
    }).done(function(data) {
        skip = false;
        poll_timeout = setTimeout(poll_events, 1000);
    }).fail(function(data) {
        alert(data.responseJSON["msg"]);
        skip = false;
        poll_timeout = setTimeout(poll_events, 100);
    });
    return true;
}

function switch_status(dev, sw) {
    var ison = devices[dev].switches[sw].status == "on";
    var ret = '';
    ret += '<li class="collection-item grey darken-2">';
    ret += '  <div class="switch">';
    ret += devices[dev].name + " - " + devices[dev].switches[sw].name;
    ret += '    <label class="pull-right">';
    ret += '      Off';
    ret += '      <input type="checkbox" class="swlever" ' + (ison? 'checked="checked"' : '') + ' />';
    ret += '      <span class="lever" onclick="toggle_switch(' + dev + ', ' + sw + ')"></span>';
    ret += '      On';
    ret += '    </label>';
    ret += '  </div>';
    ret += '</li>';
    return ret;
}

function poll_events() {
    clearTimeout(poll_timeout);
    poll_timeout = setTimeout(poll_events, 1000);
    var req_data = {
        start: (+ new Date()) / 1000 - 30*60,
        stop: (+ new Date()) / 1000 + 30 /* ensure nothing bad happens with clock skew */
    };
    $.ajax({
        type: "POST",
        url: "/api/events",
        data: JSON.stringify(req_data),
        contentType: "application/json; charset=utf-8",
        dataType: "json",
    }).done(function(data) {
        if (skip) return;
        data.sort(function(a, b) {
            return a["date_received"] > b["date_received"];
        });
        var changed = false;
        for (var ev in data) {
            var dev = data[ev].node_id;
            if (devices[dev] == undefined) {
                if (unknown_devices[dev] == undefined) {
                    unknown_devices[dev] = true;
                    changed = true;
                }
                continue;
            }
            else {
                if (unknown_devices[dev] !== undefined) {
                    changed = true;
                    delete unknown_devices[dev];
                }
            }
            if (data[ev].kind.SwitchIsOn !== undefined) {
                if (devices[dev].switches[data[ev].kind.SwitchIsOn] !== undefined)
                    devices[dev].switches[data[ev].kind.SwitchIsOn].status = "on";
            } else if (data[ev].kind.SwitchIsOff !== undefined) {
                if (devices[dev].switches[data[ev].kind.SwitchIsOff] !== undefined)
                    devices[dev].switches[data[ev].kind.SwitchIsOff].status = "off";
            }
        }
        if (Object.keys(unknown_devices).length > 0) {
            if (changed) {
                $('#adddevice').removeClass('hidden');
                var opts = '';
                for (var dev in unknown_devices) {
                    opts += '    <option value="' + dev + '">' + dev + '</option>';
                }
                $('#newdevid').html(opts);
                $('#newdevid').material_select();
                Materialize.updateTextFields();
            }
        } else {
            $('#adddevice').addClass('hidden');
        }
        var html = '<ul class="collection">';
        html += '     <li class="collection-item turnoff grey darken-3">';
        html += '       <button type="button" class="waves-effect waves-light btn blue" onclick="turnoff()" >Turn off everything</button>';
        html += '     </li>';
        for (var dev in devices) {
            for (var sw in devices[dev].switches) {
                html += switch_status(dev, sw);
            }
        }
        html += '</ul>';
        $("#status").html(html);
    });
}

function load_config() {
    start_loading();
    $.ajax({
        type: "GET",
        url: "/api/config"
    }).done(function(data) {
        devices = data.devices;
        for (var d in devices) {
            for (var s in devices[d].switches) {
                devices[d].switches[s].status = "unknown";
            }
        }
        poll_events();
        $("#config").html(gen_config_page());
        Materialize.updateTextFields();
        $('.collapsible').collapsible();
        $('.modal').modal();
        $('select').material_select();
        $('.switchform').submit(function(e) {
            e.preventDefault();
            var data = $(this).serializeArray();
            var req_data = {
                buttons: [],
                pirs: []
            };
            var id = undefined;
            var switch_id = undefined;
            for (var i in data) {
                if (data[i].name == "name" || data[i].name == "pir_time") {
                    req_data[data[i].name] = data[i].value;
                } else if (data[i].name == "device_id") {
                    id = parseInt(data[i].value);
                } else if (data[i].name == "switch_id") {
                    switch_id = parseInt(data[i].value);
                } else {
                    req_data[data[i].name].push(JSON.parse(data[i].value));
                }
            }
            $.ajax({
                type: "POST",
                url: "/api/config/" + id + "/" + switch_id,
                data: JSON.stringify(req_data),
                contentType: "application/json; charset=utf-8",
                dataType: "json",
            }).done(function(_) {
                load_config();
            }).fail(function(data) {
                alert(data.responseJSON["msg"]);
            });
            return false;
        });
        $('.configform').submit(function(e) {
            e.preventDefault();
            var data = $(this).serializeArray();
            var req_data = {};
            for (var i in data) {
                req_data[data[i].name] = data[i].value;
            }
            req_data["id"] = parseInt(req_data["id"], 10);
            req_data["num_buttons"] = parseInt(req_data["num_buttons"], 10);
            req_data["num_pirs"] = parseInt(req_data["num_pirs"], 10);
            req_data["battery_powered"] = req_data["battery_powered"] == "on";
            if (devices[req_data["id"]] === undefined) {
                req_data["switches"] = {};
            } else {
                req_data["switches"] = devices[req_data["id"]]["switches"];
                delete req_data["switches"]["status"];
            }
            $.ajax({
                type: "POST",
                url: "/api/config/" + req_data["id"],
                data: JSON.stringify(req_data),
                contentType: "application/json; charset=utf-8",
                dataType: "json",
            }).done(function(_) {
                load_config();
            }).fail(function(data) {
                alert(data.responseJSON["msg"]);
            });
            return false;
        });
        stop_loading();
    });
}

$(document).ready(function() {
    load_config();
});
