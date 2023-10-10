Ckylark
=======

`Ckylark` - A latent-annotated probabilistic context-free grammar (LAPCFG) parser.


About
-----

This software generates the phrase structure of given input
sentence using latent annotated probabilistic context-free
grammar (LAPCFG) model proposed by
[Petrov et al., 2006]
[Petrov & Klein, 2007a]
[Petrov & Klein, 2007b].

Since original LAPCFG Parser sometimes makes failed parses in
parse-time, `Ckylark` avoids this problem using
below approaches:
* Using probabilities of unknown words for parse-time
  smoothing.
* Rollbacking coarse grammar if parsing failed.

If you want to read more, or cite Ckylark when you use it in a paper,
please reference the following paper:

* Yusuke Oda, Graham Neubig, Sakriani Sakti, Tomoki Toda, Satoshi Nakamura. Ckylark: A More Robust PCFG-LA Parser. Proceedings of NAACL: Demo Track. 2015.

`Ckylark` is a portmanteau of both "CKY" and "skylark."


Install
-------

You need following tools to build Ckylark.
* `GCC 4.7` or later
* `Boost 1.49` or later
* `autotools`

You simply run below:

    cd /path/to/Ckylark
    autoreconf -i
    ./configure
    make
    (sudo) make install


Usage
-----

For simply use, you can type below command to parse your
sentences:

    src/bin/ckylark --model (model prefix) < (your word-segmented corpus)

`--model` requires the prefix of model file like `data/wsj` in
this repository.
(`data/wsj` is English model. if you need to parse Japanese
sentences, use `data/jdc` instead)

For example,

    $ echo "This is a pen ." | ckylark --model data/wsj
    ( (S (NP (DT This)) (VP (VBZ is) (NP (DT a) (NN pen))) (. .)) )

`Ckylark` uses the text dump files of original `Berkeley Parser`
models.

You can also use your original models made by `GrammarTrainer`
and `WriteGrammarToTextFiles` of `Berkeley Parser`.

And you can also use the pre-trained models listed below:

[Ckylark Models (site language; Japanese)](https://www.predicate.jp/tools/ckylark/model_ja)

If you want to see all options, please type below:

    src/bin/ckylark --help


Contributors
------------

* Yusuke Oda (@odashi) - Most coding
* Koichi Akabe (@vbkaisetsu)
* Graham Neubig (@neubig)

We are counting more contributions from you.


Official Site
-------------

[Ckylark | Yusuke Oda](http://odaemon.com/?page=tools_ckylark "Ckylark | Yusuke Oda")


Contact
-------

If you find an issue, please contact Y.Oda
* yus.takara (at) gmail.com
* @odashi_t on Twitter

