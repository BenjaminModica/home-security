const offBtn = document.getElementById('alarm-off');
const onBtn = document.getElementById('alarm-on');

offBtn.addEventListener('click', () => sendHttpRequest('http://192.168.1.239/alarm?data=0', 'alarm-off'));
onBtn.addEventListener('click', () => sendHttpRequest('http://192.168.1.239/alarm?data=1', 'alarm-on'));

function sendHttpRequest  (url, data) {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', url);

    xhr.send(data);
};

