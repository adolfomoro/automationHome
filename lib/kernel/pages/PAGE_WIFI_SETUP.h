const char PAGE_WIFI_SETUP[] PROGMEM = R"=====(
<div class="container mt-3">
    <h3 class="text-center">Wifi Settings</h3>
    <form id="networkEdit">
        <div class="mb-3 mt-3">
            <label for="ssid">SSID:</label>
            <input type="text" class="form-control" id="SSID" name="SSID" maxlength="25">
        </div>
        <div class="mb-3 mt-3">
            <label for="pwd">Password:</label>
            <input type="text" class="form-control" id="PWD" name="PWD" maxlength="25">
        </div>
        <button type="submit" class="btn btn-success">ADD</button>
    </form>
    <div class="table-responsive mt-4">
        <table class="table">
            <thead>
                <tr>
                    <th scope="col">SSID</th>
                    <th class="col-auto" scope="col">Options</th>
                </tr>
            </thead>
            <tbody id="tableNetworks">
            </tbody>
        </table>
    </div>
</div>
<script>
    var networks = [%NETWORKS_LOAD%];

    function reloadItens() {
        $("#tableNetworks").empty();
        for (var i = 0; i < networks.length; i++) {
            var trHTML = '';
            trHTML += '<tr>';
            trHTML += '<td>' + networks[i].SSID + '</td>';
            trHTML += '<td><button onclick="deletWifi(\'' + networks[i].SSID +
                '\')" type="button" class="btn btn-danger btn-sm">DELETE</button></td>';
            trHTML += "</tr>";
            $("#tableNetworks").append(trHTML);
        }
    }

    $("#networkEdit").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/network/add',
            type: 'GET',
            data: {
                SSID: $("#SSID").val(),
                PWD: $("#PWD").val()
            },
            success: function (response) {
                networks = response;
                $("#SSID").val('');
                $("#PWD").val('');
            },
            error: function (e) {
                if (typeof e.responseText !== 'undefined') {
                    alert(e.responseText);
                } else {
                    alert("%ERROR_REQUEST%");
                }
            },
            complete() {
                reloadItens();
            }
        });
    });

    function deletWifi(ssidReceive) {
        if (window.confirm("Confirm delete Network?")) {
            $.ajax({
                url: '/network/delete',
                type: 'GET',
                data: {
                    SSID: ssidReceive
                },
                success: function (response) {
                    networks = response;
                },
                error: function (e) {
                    if (typeof e.responseText !== 'undefined') {
                        alert(e.responseText);
                    } else {
                        alert("%ERROR_REQUEST%");
                    }
                },
                complete() {
                    reloadItens();
                }
            });
        }
    }
    reloadItens();
</script>
)=====";