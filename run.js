const _easyexec = require('./lib/easyexec');

//if(_easyexec.chroot("./build") == -1)
//   console.log("error");
//_easyexec.chdir("/");
_easyexec.execvp(['/bin/ls','-lrth']);

console.warn(_easyexec);
