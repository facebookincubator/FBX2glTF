# FBX2glTF Regression Suite

It's imperative that as we add new functionality to FBX2glTF and refactor code,
that old behaviour does not change underfoot in unexpected and detrimental ways.

This is a very simple first beginnings of a test suite for converting FBX files
and ensuring the consistency of the GLB. At present all we do is make sure that
the generated GLB is valid. We will have to add other integrity checks here for
the tests to accomplish what we intend.

We will also want to test the same file with a variety of command line switches,
e.g. with Draco compression and different materials.

## Setting up

We recommend using Yarn rather than NPM. Installation instructions can be found
[here](https://yarnpkg.com/lang/en/docs/install/).

> yarn install

## Running tests

> yarn test

