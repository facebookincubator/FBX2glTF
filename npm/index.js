const childProcess = require('child_process');
const os = require('os');
const path = require('path');

const binaries = {
  'darwin': `bin/darwin/Fbx2Gtlf`,
  'linux': `bin/linux/Fbx2Gtlf`,
  'win32': `bin\windows\Fbx2Gtlf.exe`,
};

function fbx2glb(srcFile, destFile, cwd) {
  return new Promise((resolve, reject) => {
    let script = os.type() === 'Windows_NT' ? 'fbx2glb.bat' : 'fbx2glb.sh';
    let child;
    try {
      let opts = {};
      cwd && (opts.cwd = cwd);
      child = childProcess.spawn(
        path.join(__dirname, 'bin', script),
        [ srcFile, destFile ],
        opts
      );
    } catch (error) {
      reject(error);
      return;
    }
    let output = '';
    child.stdout.on('data', (data) => output += data);
    child.stderr.on('data', (data) => output += data);
    child.on('error', reject);
    child.on('close', code => {
      // non-zero exit code is failure
      if (code != 0) {
        reject(new Error(`Script ${script} output:\n` +
                         (output.length ? output : "<none>")));
      } else {
        resolve(destFile);
      }
    });
  });
}

module.exports = fbx2glb;
