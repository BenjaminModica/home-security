const offBtn = document.getElementById('alarm-off');
const onBtn = document.getElementById('alarm-on');

offBtn.addEventListener('click', () => sendHttpRequest('http://192.168.1.239:8080/alarm?data=0'));
onBtn.addEventListener('click', () => sendHttpRequest('http://192.168.1.239:8080/alarm?data=1'));

setInterval(() => sendHttpRequest('http://192.168.1.239:8080/status'), 1000);

function sendHttpRequest  (url) {
    const xhr = new XMLHttpRequest();
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

