let authed=false;

function login(){
  authed=true;
  document.getElementById('login').classList.add('hidden');
  document.getElementById('panel').classList.remove('hidden');
  loadStatus();
}

function loadStatus(){
  fetch('/status').then(r=>r.json()).then(j=>{
    document.getElementById('version').innerText=j.version;
    document.getElementById('uptime').innerText=j.uptime;
  }).catch(()=>{});
}

function poll(){
  fetch('/ota-progress').then(r=>r.json()).then(j=>{
    let p=j.progress||0;
    document.getElementById('progress').style.width=p+'%';
    document.getElementById('progress-text').innerText=p+'%';
    document.getElementById('badge').innerText=p<100?'UPDATING':'REBOOT';
    if(p<100)setTimeout(poll,500);
  });
}

function upload(){
  let f=document.getElementById('file').files[0];
  if(!f){
    alert('Select firmware');
    document.getElementById('login-msg').innerText = '⚠️ Please select a firmware file.';
    return;
  }
  let d=new FormData(); d.append('update',f);
  fetch('/update',{method:'POST',body:d})
    .then(response => {
      if (response.ok) {
        response.text().then(body => {
          console.log('Server response:', body);
          document.getElementById('status-msg').classList.remove('hidden');
          setTimeout(() => location.reload(), 3000);
        });
        poll();
      } else {
        document.getElementById('login-msg').innerText = '❌ Upload failed. Please try again.';
      }
    })
    .catch(err => {
      console.error('Upload error:', err);
      document.getElementById('login-msg').innerText = '❌ Network error during upload.';
    });
}

function updateFromURL(){
  let u=document.getElementById('url').value;
  if(!u)return alert('Enter URL');
  fetch('/update-url',{method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'url='+encodeURIComponent(u)
  });
  poll();
}
