#include <iostream>

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>