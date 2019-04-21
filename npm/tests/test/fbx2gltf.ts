/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

import {assert, expect} from 'chai';
import * as fbx2gltf from 'fbx2gltf';
import {readFileSync} from 'fs';
import {validateBytes} from 'gltf-validator';
import * as path from 'path';
import * as tmp from 'tmp';

interface Model {
  path: string;
  ignoredIssues?: Array<string>;
  args?: Array<string>;
}

const MODELS: Array<Model> = [
  {path : 'fromFacebook/Jon/jon_morph'},
  {
    path : 'fromFacebook/Jon/troll-final',
    ignoredIssues : [ 'ACCESSOR_NON_UNIT' ],
  },
  {path : 'fromFacebook/Natalie/GlitchRobot'},
  {path : 'fromFacebook/Ocean/blackvan/blackvan_with_windows'},
  {
    path : 'fromFacebook/Ocean/zell_van_vertex_color',
    args : [ '--draco' ],
    ignoredIssues : [ 'UNSUPPORTED_EXTENSION' ],
  },
  {path : 'fromFacebook/RAZ/RAZ_ape', args : [ '--long-indices=always' ]},
  {path : 'fromFbxSDK/Box'},
  {
    path : 'fromFbxSDK/Humanoid',
    ignoredIssues : [ 'UNSUPPORTED_EXTENSION' ],
  },
  {
    path : 'fromFbxSDK/Camera',
    ignoredIssues : [ 'UNSUPPORTED_EXTENSION' ],
  },
  {path : 'fromFbxSDK/Normals'},
  {path : 'fromGltfSamples/BoxVertexColors/BoxVertexColors', args : [ '--khr-materials-unlit' ]},
  {path : 'fromGltfSamples/WaterBottle/NewWaterBottle'},
];

const CONVERSION_TIMEOUT_MS = 50000;

describe('FBX2glTF', () => {
  const tmpobj = tmp.dirSync();
  for (let model of MODELS) {
    const modelName = path.basename(model.path);
    describe('Model: ' + modelName, () => {
      const fbxPath = path.join('models', model.path + '.fbx');
      let glbBytes;
      it('should convert fbx to glb', async () => {
        const glbPath = path.join(tmpobj.name, modelName + '.glb');

        try {
          const destPath = await fbx2gltf(fbxPath, glbPath, model.args || []);
          assert.isNotNull(destPath);
          glbBytes = readFileSync(destPath);
        } catch (err) {
          throw new Error('Conversion failed: ' + err);
        }
      }).timeout(CONVERSION_TIMEOUT_MS);

      if (!glbBytes) {
        return;
      }
      it('resulting glb should be valid', async () => {
        try {
          let options = <any>{};
          if (model.ignoredIssues) {
            options.ignoredIssues = model.ignoredIssues;
          }
          const report = await validateBytes(glbBytes, options);
          expect(report.issues.numErrors).to.equal(0);
          expect(report.issues.numWarnings).to.equal(0);

        } catch (err) {
          throw new Error('Validation failed: ' + err);
        }
      });
    });
  }
  console.log('GLB files may be inspected in: ' + tmpobj.name);
});
