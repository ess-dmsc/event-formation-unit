/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for registering stat counters and associating them
 *  with names.
 */

 #pragma once

 #include <string>
 #include <vector>
 #include <cinttypes>

 class StatTuple {
 public:
   std::string name;
   int64_t * counter;
 };

 class NewStats {
 public:
   /** @todo document */
   NewStats();

   /** @todo document */
   int create(std::string statname, int64_t * counter);

   /** @todo document */
   size_t size();

   /** @todo document */
   std::string & name(size_t index);

   /** @todo document */
   int64_t value(size_t index);

 private:
   std::vector<StatTuple *> stats;
   std::string nostat{""};
 };
