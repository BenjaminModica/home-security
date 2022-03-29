const char webpage[] =
R"=====(
<!DOCTYPE html>      
<html>      
<head>
    <meta charset="UTF-8">
    <!--<script src="xhr.js" defer></script>-->      
    <title>Home Control Panel</title>
</head>      
<!--HTML-->
<body style="text-align: center;">      
    <h1> Control Panel </h1>                                          
    <br>      
    <button style="margin: 10px;" id="alarm-off">Turn Off Alarm</button>      
    <br>      
    <button style="margin: 10px;" id="alarm-on">Set Alarm</button>   
    <br>
    <h2>Alarm status:</h2>
    <h3 id="status">%Status will shown here%</h3>
<!--Javascript-->
<script>
const offBtn = document.getElementById('alarm-off');
const onBtn = document.getElementById('alarm-on');

offBtn.addEventListener('click', () => sendHttpRequest('http://188.151.232.204:65477/alarm?data=alarmOff'));
onBtn.addEventListener('click', () => sendHttpRequest('http://188.151.232.204:65477/alarm?data=alarmOn'));

setInterval(() => sendHttpRequest('http://188.151.232.204:65477/status'), 3000)

function sendHttpRequest  (url) {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', url);

    //Listen to response
    xhr.onload = () => {
        data = xhr.response;
        console.log(data);

        //Update status on page
        const element = document.getElementById("status");
        if (data == 'alarmOff') {
            element.innerHTML = "OFF";
        } else if (data == 'alarmOn') {
            element.innerHTML = "READY";
        } else if (data == 'alarmTrig') {
            element.innerHTML = "ALARM TRIGGERED";
        } else {
            element.innerHTML = "Error";
        }
    }

    xhr.send();
};
</script>
</body>      
</html>
)=====";
