const char PAGE_RESET[] PROGMEM = R"=====(
<div class="container mt-3">
    <h3 class="text-center">Reset Board</h3>
    <form class="mt-4">
        <div class="mb-3 mt-3">
            <label for="pwd">Password:</label>
            <input type="password" class="form-control" id="pwd" name="pwd" maxlength="15">
        </div>
        <div class="mb-3 mt-3">
            <label for="pwd_confirm">Password Confirm:</label>
            <input type="password" class="form-control" id="pwd_confirm" name="pwd_confirm" maxlength="15">
        </div>
        <button type="submit" class="btn btn-success">Reset</button>
    </form>
</div>
<script>
    $("form").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/reset_c',
            type: 'GET',
            data: {
                PWD: $("#pwd").val(),
                PWD_CONFIRM: $("#pwd_confirm").val()
            },
            success: function (response) {
                if (response == "true") {
                    alert("Success!");
                    window.location.href = '/restart';
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