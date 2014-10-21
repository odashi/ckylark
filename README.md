Ckylark
=======

`Ckylark` - CKY-based latent annotated rule ekstractor


About
-----

This software generates the phrase structure of given input
sentence using latent annotated probabilistic context-free
grammar (LAPCFG) model proposed by [Petrov et al., 2006]
[Petrov & Klein, 2007].

Since original LAPCFG Parser sometimes makes failed parses in
parse-time, `Ckylark` avoids this problem using
below approaches:
* Using probabilities of unknown words for parse-time
  smoothing.
* Rollbacking coarse grammar if parsing failed.

`Ckylark` is a portmanteau of both "CKY" and "skylark."


Install
-------

You need `autotools` to build Ckylark.
You simply run below:

    cd /path/to/Ckylark
    autoreconf -i
    ./configure
    make


Usage
-----

When you use the bundled model (`wsj` or `jdc`), decompress all
files in `model` directory beforehand.

For simply use, you can type below command to parse your
sentences:

    src/ckylark --model (model prefix) < (your word-segmented corpus)

`--model` requires the prefix of model file like `model/wsj` in
this repository.
(`model/wsj` is English model. if you need to parse Japanese
sentences, use `model/jdc` instead)

For example,

    $ echo "This is a pen ." | src/ckylark --model model/wsj
    ( (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN pen))) (. .)) )

`Ckylark` uses the text dump files of original `Berkeley Parser`
models.
You can also use your original models made by `GrammarTrainer`
and `WriteGrammarToTextFiles` of `Berkeley Parser`.

If you want to see all options, please type below:

    src/ckylark --help


Contributors
------------

* Yusuke Oda (@odashi) - Most coding
* Koichi Akabe (@vbkaisetsu)
* Graham Neubig (@neubig)

We are counting more contributions from you.


Contact
-------

If you find an issue, please contact Y.Oda
* yus.takara (at) gmail.com
* @odashi_t on Twitter

