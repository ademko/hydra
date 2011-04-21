
/*
 * Copyright (c) 2011 Aleksander B. Demko
 * This source code is distributed under the MIT license.
 * See the accompanying file LICENSE.MIT.txt for details.
 */

#ifndef __INCLUDED_WEXUS_CONTROLLER_H__
#define __INCLUDED_WEXUS_CONTROLLER_H__

namespace wexus
{
  class Controller;
}

/**
 * Base class for all user defined controllers.
 * Controller is the C in MVC. They are instantied
 * per request, with the appripriate sub message being called.
 *
 * @author Aleksander Demko
 */ 
class wexus::Controller
{
  public:
    virtual ~Controller();

  protected:
    /// constructor
    Controller(void);
};

#endif

