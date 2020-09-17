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

#include <graphene/protocol/operations.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {
    
   class event_group_object;

   class event_group_create_evaluator : public evaluator<event_group_create_evaluator>
   {
      public:
         typedef event_group_create_operation operation_type;

         void_result do_evaluate( const event_group_create_operation& o );
         object_id_type do_apply( const event_group_create_operation& o );

      private:
         sport_id_type sport_id;
   };

   class event_group_update_evaluator : public evaluator<event_group_update_evaluator>
   {
      public:
         typedef event_group_update_operation operation_type;

         void_result do_evaluate( const event_group_update_operation& o );
         void_result do_apply( const event_group_update_operation& o );

      private:
         sport_id_type sport_id;
   };
    
   class event_group_delete_evaluator : public evaluator<event_group_delete_evaluator>
   {
   public:
       typedef event_group_delete_operation operation_type;
       
       void_result do_evaluate( const event_group_delete_operation& o );
       void_result do_apply( const event_group_delete_operation& o );
       
   private:
       const event_group_object* _event_group = nullptr;
   };
} } // graphene::chain
