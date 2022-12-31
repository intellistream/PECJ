//
// Created by tony on 29/12/22.
//

#include <TestBench/DataLoaderTable.h>
#include <TestBench/RandomDataLoader.h>
#include <TestBench/FileDataLoader.h>
OoOJoin::DataLoaderTable::DataLoaderTable() {
  loaderMap["random"] = newRandomDataLoader();
  loaderMap["file"] = newFileDataLoader();
}