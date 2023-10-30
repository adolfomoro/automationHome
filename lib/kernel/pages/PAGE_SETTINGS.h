const char PAGE_SETTINGS[] PROGMEM = R"=====(
<div class="container mt-3">
    <h3 class="text-center">Settings</h3>
    <form>
        <div class="mb-3 mt-3">
            <label for="board_name">Board Name: *</label>
            <input type="text" class="form-control" id="board_name" name="board_name" maxlength="15" value="%BOARD_NAME%">
        </div>
        <div class="mb-3 mt-4">
            <label for="server_ip">Server IP: *</label>
            <select class="form-select" id="server_type" name="server_type">
            <option value="" selected>Selection Option</option>
            <option value="MQTT">MQTT</option>
            </select>
        </div>
        <div class="mb-3 mt-3">
            <label for="server_ip">Server IP: *</label>
            <input type="text" class="form-control" id="server_ip" name="server_ip" maxlength="15" value="%SERVER_IP%">
        </div>
        <div class="mb-3 mt-3">
            <label for="server_port">Server Port: *</label>
            <input type="number" class="form-control" id="server_port" name="server_port" maxlength="5" value="%SERVER_PORT%">
        </div>
        <div class="mb-3 mt-3">
            <label for="server_user">Server User: *</label>
            <input type="text" class="form-control" id="server_user" name="server_user" maxlength="15" value="%SERVER_USER%">
        </div>
        <div class="mb-3 mt-3">
            <label for="server_pwd">Server Password:</label>
            <input type="text" class="form-control" id="server_pwd" name="server_pwd" maxlength="15" value="%SERVER_PWD%">
        </div>
        <button type="submit" class="btn btn-success">Save</button>
    </form>
</div>
<script>
    $('#server_ip').mask('0ZZ.0ZZ.0ZZ.0ZZ', { translation: { 'Z': { pattern: /[0-9]/, optional: true } } });
    $("#server_type").val("%SERVER_TYPE%");
    $("form").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/set_settings',
            type: 'GET',
            data: {
                board_name: $("#board_name").val(),
                server_type: $("#server_type").val(),
                server_ip: $("#server_ip").val(),
                server_port: $("#server_port").val(),
                server_user: $("#server_user").val(),
                server_pwd: $("#server_pwd").val()
            },
            success: function (response) {
                console.log(response);
                if (response == "true") {
                    alert("Success!");
                } else {
                    alert(response);
                }
            },
            error: function (e) {
                console.log(e);
                if (typeof e.responseText !== 'undefined') {
                    alert(e.responseText)
                }else{
                    alert('Error');
                }
            }
        });
    });
</script>
)=====";