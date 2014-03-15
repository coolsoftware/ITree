ITree

Copyright 2014 CoolSoftware
http://blog.coolsoftware.ru/

itree is a C++ class that implements Interval Tree functionality.

It was originally written by Yogi Dandass, January 01, 2000.
I got it from http://www.drdobbs.com/cpp/interval-trees/184401179

I have fixed some major bugs and significally improved performance of constructing Interval Tree structure.

itree это C++ класс, который реализует функциональность т.н. "Дерева Интервалов" (Interval Tree).
Оригинальную версию написал Yogi Dandass в январе 2000 (http://www.drdobbs.com/cpp/interval-trees/184401179).

Я добавил некоторые недостающие (ommited) определения, исправил два бага, приводивших к ошибке index out of range, а также переработал и значительно ускорил конструирование дерева.

TestITree.sln - это тестовое приложение (для Visual Studio 2010), которое позволяет оценить скорость и корректность работы класса itree.

Свои отзывы и предложения можете присылать на vitaly@coolsoftware.ru