#include <graphene/chain/custom_permission_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/custom_permission_object.hpp>
#include <graphene/chain/hardfork.hpp>

namespace graphene
{
namespace chain
{

void_result create_custom_permission_evaluator::do_evaluate(const custom_permission_create_operation &op)
{
   try
   {
      const database &d = db();
      auto now = d.head_block_time();
      FC_ASSERT(now >= HARDFORK_RBAC_TIME, "Not allowed until RBAC HF");
      op.owner_account(d);
      for (const auto &account_weight_pair : op.auth.account_auths)
      {
         account_weight_pair.first(d);
      }

      const auto &pindex = d.get_index_type<custom_permission_index>().indices().get<by_account_and_permission>();
      auto pitr = pindex.find(boost::make_tuple(op.owner_account, op.permission_name));
      FC_ASSERT(pitr == pindex.end(), "Permission name already exists for the given account");
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

object_id_type create_custom_permission_evaluator::do_apply(const custom_permission_create_operation &op)
{
   try
   {
      database &d = db();
      return d.create<custom_permission_object>([&op](custom_permission_object &obj) mutable {
                 obj.account = op.owner_account;
                 obj.permission_name = op.permission_name;
                 obj.auth = op.auth;
              }).id;
   }
   FC_CAPTURE_AND_RETHROW((op))
}

void_result update_custom_permission_evaluator::do_evaluate(const custom_permission_update_operation &op)
{
   try
   {
      const database &d = db();
      auto now = d.head_block_time();
      FC_ASSERT(now >= HARDFORK_RBAC_TIME, "Not allowed until RBAC HF");
      op.owner_account(d);
      const custom_permission_object& pobj = op.permission_id(d);
      FC_ASSERT(pobj.account == op.owner_account, "Only owner account can update permission object");
      if (op.new_auth)
      {
         FC_ASSERT(!(*op.new_auth == pobj.auth), "New authority provided is not different from old authority");
         for (const auto &account_weight_pair : op.new_auth->account_auths)
         {
            account_weight_pair.first(d);
         }
      }
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

object_id_type update_custom_permission_evaluator::do_apply(const custom_permission_update_operation &op)
{
   try
   {
      database &d = db();
      const custom_permission_object& pobj = op.permission_id(d);
      d.modify(pobj, [&op](custom_permission_object &obj) {
         if (op.new_auth)
            obj.auth = *op.new_auth;
      });

      return op.permission_id;
   }
   FC_CAPTURE_AND_RETHROW((op))
}

void_result delete_custom_permission_evaluator::do_evaluate(const custom_permission_delete_operation &op)
{
   try
   {
      const database &d = db();
      auto now = d.head_block_time();
      FC_ASSERT(now >= HARDFORK_RBAC_TIME, "Not allowed until RBAC HF");
      op.owner_account(d);
      const custom_permission_object& pobj = op.permission_id(d);
      FC_ASSERT(pobj.account == op.owner_account, "Only owner account can delete permission object");
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

void_result delete_custom_permission_evaluator::do_apply(const custom_permission_delete_operation &op)
{
   try
   {
      database &d = db();
      const custom_permission_object& pobj = op.permission_id(d);
      // TODO: Remove all the custom_account_authority_object linked to this permission object.
      d.remove(pobj);
      return void_result();
   }
   FC_CAPTURE_AND_RETHROW((op))
}

} // namespace chain
} // namespace graphene