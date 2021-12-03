const offBtn = document.getElementById('alarm-off');
const onBtn = document.getElementById('alarm-on');

offBtn.addEventListener('click', () => sendHttpRequest('http://83.251.161.248:255/alarm?data=alarmOff'));
onBtn.addEventListener('click', () => sendHttpRequest('http://83.251.161.248:255/alarm?data=alarmOn'));

setInterval(() => sendHttpRequest('http://83.251.161.248:255/status'), 3000);

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

