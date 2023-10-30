const char PAGE_SIGN_UP_USER[] PROGMEM = R"=====(
<div class="container mt-3">
    <h3 class="text-center">Security</h3>
    <form>
        <div class="mb-3 mt-3">
            <label for="user">User:</label>
            <input type="text" class="form-control" id="user" name="user" maxlength="15">
        </div>
        <div class="mb-3 mt-3">
            <label for="pwd">Password:</label>
            <input type="password" class="form-control" id="pwd" name="pwd" maxlength="15">
        </div>
        <button type="submit" class="btn btn-success">Save</button>
    </form>
</div>
<script>
    $("form").submit(function (e) {
        e.preventDefault();
        $.ajax({
            url: '/sign_up_user',
            type: 'GET',
            data: {
                USER: $("#user").val(),
                PWD: $("#pwd").val()
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