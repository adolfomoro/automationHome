const char PAGE_SECURITY[] PROGMEM = R"=====(
<div class="container mt-3">
    <h3 class="text-center">Security</h3>
    <form id="change_user">
        <div class="mb-3 mt-3">
            <label for="user">User:</label>
            <input type="text" class="form-control" id="user" name="user" maxlength="15">
        </div>
        <button type="submit" class="btn btn-success">Save</button>
    </form>
    <form class="mt-4" id="change_pwd">
        <div class="mb-3 mt-3">
            <label for="pwd">Password:</label>
            <input type="password" class="form-control" id="pwd" name="pwd" maxlength="15">
        </div>
        <div class="mb-3 mt-3">
            <label for="pwd_confirm">Password Confirm:</label>
            <input type="password" class="form-control" id="pwd_confirm" name="pwd_confirm" maxlength="15">
        </div>
        <button type="submit" class="btn btn-success">Save</button>
    </form>
</div>
<script>
    $("form#change_user").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/change_user',
            type: 'GET',
            data: {
                user: $("#user").val()
            },
            success: function (response) {
                if (response == "true") {
                    alert("Success!");
                    window.location.href = '/';
                } else {
                    alert(response);
                }
            },
            error: function (e) {
                if (typeof e.responseText !== 'undefined') {
                    alert(e.responseText)
                }else{
                    alert('Error');
                }
            }
        });
    });
    $("form#change_pwd").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/change_pwd',
            type: 'GET',
            data: {
                pwd: $("#pwd").val(),
                pwd_confirm: $("#pwd_confirm").val()
            },
            success: function (response) {
                if (response == "true") {
                    alert("Success!");
                    window.location.href = '/';
                } else {
                    alert(response);
                }
            },
            error: function (e) {
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