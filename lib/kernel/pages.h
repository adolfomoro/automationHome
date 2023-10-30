const char PAGE_HEADER[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <title>%TITLE%</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="\bootstrap.min.css" rel="stylesheet">
    <script src="\jquery.min.js"></script>
    <script src="\jquery.mask.min.js"></script>
    <style>
        body {
            background-color: #343a40;
        }
    </style>
</head>

<body>
    <div class="col-lg-5 col-md-5 col-sm-5 container justify-content-center">
        <div class="col p-4 mt-3" style="background:#fff">
            %PAGE%
        </div>
    </div>
</body>

</html>
)=====";

const char INDEX_PAGE[] PROGMEM = R"=====(
<h3 class="text-center">%COMPANY%</h3>
<div class="mb-4">
    <div class="row">
        <div class="col">
            Wifi Status:
        </div>
        <div class="col">
            %WIFI_STATUS%
        </div>
    </div>
    <div class="row">
        <div class="col">
            Server Status:
        </div>
        <div class="col">
            %SERVER_STATUS%
        </div>
    </div>
</div>
<div class="d-grid gap-2 mt-4">
    <a href="/security" type="button" class="btn btn-primary btn-block">Security System</a>
    <a href="/network" type="button" class="btn btn-primary btn-block">Wifi Settings</a>
    <a href="/settings" type="button" class="btn btn-primary btn-block">Settings</a>
    <a href="/restart" type="button" class="btn btn-danger btn-block mt-4 mb-1">Restart Board</a>
    <a href="/reset" type="button" class="btn btn-danger btn-block">Reset Settings</a>
</div>
)=====";

const char PAGE_RESTART_BOARD[] PROGMEM = R"=====(
<h3 id="message"></h3>
<script>
    message = "Wait, restart in ";
    distance = 15;
    $("#message").text(message + " " + distance);
    var x = setInterval(function () {
        distance--;
        $("#message").text(message + " " + distance);
        if (distance <= 0) {
            clearInterval(x);
            window.location.href = '/';
        }
    }, 1000);
    window.addEventListener('load', function () {
        var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "/restart_c", true);
        xhttp.send();
    })
</script>
)=====";