/*
 * Copyright (c) 2018 Peerplays Blockchain Standards Association, and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/event_object.hpp>

#include <graphene/protocol/operations.hpp>

namespace graphene { namespace chain {

   class event_create_evaluator : public evaluator<event_create_evaluator>
   {
      public:
         typedef event_create_operation operation_type;

         void_result do_evaluate( const event_create_operation& o );
         object_id_type do_apply( const event_create_operation& o );
      private:
         event_group_id_type event_group_id;
   };

   class event_update_evaluator : public evaluator<event_update_evaluator>
   {
      public:
         typedef event_update_operation operation_type;

         void_result do_evaluate( const event_update_operation& o );
         void_result do_apply( const event_update_operation& o );
      private:
         event_group_id_type event_group_id;
   };

   class event_update_status_evaluator : public evaluator<event_update_status_evaluator>
   {
      public:
         typedef event_update_status_operation operation_type;

         void_result do_evaluate( const event_update_status_operation& o );
         void_result do_apply( const event_update_status_operation& o );
      private:
         const event_object* _event_to_update = nullptr;
   };

} } // graphene::chain
