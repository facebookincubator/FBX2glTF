import { readFileSync } from 'fs';
import * as tmp from 'tmp';
import * as path from 'path';
import * as fbx2gltf from 'fbx2gltf';
import { assert, expect } from 'chai';
import { validateBytes } from 'gltf-validator';

interface Model {
    path: string;
}

const MODELS :Array<Model> = [
    { path: 'fromFacebook/Jon/jon_morph' },
    { path: 'fromFacebook/Jon/troll-final' },
    { path: 'fromFacebook/Natalie/GlitchRobot' },
    { path: 'fromFacebook/Ocean/blackvan/blackvan_with_windows' },
    { path: 'fromFacebook/Ocean/zell_van_vertex_color' },
    { path: 'fromFacebook/RAZ/RAZ_ape' },
    { path: 'fromFbxSDK/Box' },
    { path: 'fromFbxSDK/Humanoid' },
    { path: 'fromFbxSDK/Camera' },
    { path: 'fromFbxSDK/Normals' },
    { path: 'fromGltfSamples/BoxVertexColors/BoxVertexColors' },
    { path: 'fromGltfSamples/WaterBottle/NewWaterBottle' },
];

const CONVERSION_TIMEOUT_MS = 50000;

process.env['ELECTRON_DISABLE_SECURITY_WARNINGS'] = 'true';

describe('FBX2glTF', () => {
    const tmpobj = tmp.dirSync();
    for(let model of MODELS) {
	const modelName = path.basename(model.path);
	describe('Model: ' + modelName, () => {
	    const fbxPath = path.join('models', model.path + '.fbx');
	    let glbBytes;
	    it('should convert fbx to glb', async () => {
		const glbPath = path.join(tmpobj.name, modelName + '.glb');

		try {
		    const destPath = await fbx2gltf(fbxPath, glbPath);
		    assert.isNotNull(destPath);
		    glbBytes = readFileSync(destPath);
		} catch (err) {
		    throw new Error('Conversion failed: ' + err);
		}
	    }).timeout(CONVERSION_TIMEOUT_MS);

	    it('resulting glb should be valid', async() => {
		try {
		    const report = await validateBytes(
			glbBytes, {
			    ignoredIssues: [ 'ACCESSOR_NON_UNIT' ],
			},
		    );
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
