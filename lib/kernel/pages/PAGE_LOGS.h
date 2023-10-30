const char PAGE_LOGS[] PROGMEM = R"=====(
<div class="container mt-3">
 <table class="table">
            <thead>
                <tr>
                    <th scope="col">TYPE</th>
    <th scope="col">MESSAGE</th>
    <th scope="col">DATE</th>
    <th scope="col">MILLIS</th>
    <th scope="col">START</th>
                </tr>
            </thead>
            <tbody id="mytable">
            </tbody>
        </table>
</div>
<script>
var logs;
function reloadItens() {
        $("#mytable").empty();
        for (var i = 0; i < logs.length; i++) {
            var trHTML = '';
            trHTML += '<tr>';
            trHTML += '<td>' + logs[i].type + '</td>';
            trHTML += '<td>' + logs[i].message + '</td>';
            trHTML += '<td>' + logs[i].datetime + '</td>';
            trHTML += '<td>' + logs[i].millis + '</td>';
            trHTML += '<td>' + logs[i].start + '</td>';
            trHTML += "</tr>";
            $("#mytable").append(trHTML);
            if (i > 500){
                i = logs.length;
            }
        }
    }
        $.ajax({
            url: '/log.txt',
            type: 'GET',
            success: function (response) {
                teste = response.replace(/\n*$/, "").slice(0, -2)
                teste2 = "["+teste+"]";
                logs = JSON.parse(teste2);
                reloadItens();
            },
            error: function (e) {
                if (typeof e.responseText !== 'undefined') {
                    console.log(response);
                }else{
                    alert('Error');
                }
            }
        });
</script>
)=====";