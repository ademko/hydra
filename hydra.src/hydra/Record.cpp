
/*
 * Copyright (c) 2009 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <hydra/Record.h>

const char* hydra::Record::Exception::what(void) const throw()
{
  return "hydra::Record::Exception or derived exception";
}

