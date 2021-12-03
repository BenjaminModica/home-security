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
    <button style="margin: 10px;" id="alarm-off">ALARM OFF</button>      
    <br>      
    <button style="margin: 10px;" id="alarm-on">ALARM ON</button>   
    <br>
    <h2>Alarm status:</h2>
    <h3 id="status">Status will shown here</h3>
<!--Javascript-->
<script>
const offBtn = document.getElementById('alarm-off');
const onBtn = document.getElementById('alarm-on');

offBtn.addEventListener('click', () => sendHttpRequest('http://83.251.161.248:255/alarm?data=0'));
onBtn.addEventListener('click', () => sendHttpRequest('http://83.251.161.248:255/alarm?data=1'));

setInterval(() => sendHttpRequest('http://83.251.161.248:255/status'), 10000);

function sendHttpRequest  (url) {
    const xhr = new XMLHttpRequest();168.
    xhr.open('GET', url);

    //Listen to response
    xhr.onload = () => {
        data = xhr.response;
        console.log(data);

        //Update status on page
        const element = document.getElementById("status");
        if (data == '0') {
            element.innerHTML = "OFF";
        } else if (data == '1') {
            element.innerHTML = "READY";
        } else if (data == '2') {
            element.innerHTML = "ALARM TRIPPED";
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