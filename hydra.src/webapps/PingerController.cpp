
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#include <webapps/PingerController.h>

#include <wexus/HTMLString.h>

using namespace wexus;
using namespace webapps;

//
//
// PingerController
//
//

PingerController::PingerController(void)
{ 
  registerAction<PingerController, &PingerController::index>("index");
}

void PingerController::index(void)
{
  /*output() << "from within PingerController::index() via Context<p>\n"
    "and some encoded tags (viewsource): "
    << "<inline_encoded_tag>"
    << HTMLString::encode("<encoded_tag>")
    << HTMLString("<raw_tag>")
    << "<p>\n";*/

  indexHtml();
}

