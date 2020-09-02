#include <graphene/chain/account_role_evaluator.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/account_role_object.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/rbac_hardfork_visitor.hpp>

namespace graphene
{
    namespace chain
    {

        void_result account_role_create_evaluator::do_evaluate(const account_role_create_operation &op)
        {
            try
            {
                const database &d = db();
                auto now = d.head_block_time();
                FC_ASSERT(now >= HARDFORK_NFT_TIME, "Not allowed until NFT HF");
                op.owner(d);

                rbac_operation_hardfork_visitor arvtor(now);
                for (const auto &op_type : op.allowed_operations)
                {
                    arvtor(op_type);
                }

                for (const auto &acc : op.whitelisted_accounts)
                {
                    acc(d);
                }
                return void_result();
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

        object_id_type account_role_create_evaluator::do_apply(const account_role_create_operation &op)
        {
            try
            {
                database &d = db();
                return d.create<account_role_object>([&op](account_role_object &obj) mutable {
                            obj.owner = op.owner;
                            obj.name = op.name;
                            obj.metadata = op.metadata;
                            obj.allowed_operations = op.allowed_operations;
                            obj.whitelisted_accounts = op.whitelisted_accounts;
                        })
                    .id;
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

        void_result account_role_update_evaluator::do_evaluate(const account_role_update_operation &op)
        {
            try
            {
                const database &d = db();
                auto now = d.head_block_time();
                FC_ASSERT(now >= HARDFORK_NFT_TIME, "Not allowed until NFT HF");
                op.owner(d);
                const account_role_object &aobj = op.account_role_id(d);
                FC_ASSERT(aobj.owner == op.owner, "Only owner account can update account role object");

                for (const auto &op_type : op.allowed_operations_to_remove)
                {
                    FC_ASSERT(aobj.allowed_operations.find(op_type) != aobj.allowed_operations.end(),
                              "Cannot remove non existent operation");
                }

                for (const auto &acc : op.accounts_to_remove)
                {
                    FC_ASSERT(aobj.whitelisted_accounts.find(acc) != aobj.whitelisted_accounts.end(),
                              "Cannot remove non existent account");
                }

                rbac_operation_hardfork_visitor arvtor(now);
                for (const auto &op_type : op.allowed_operations_to_add)
                {
                    arvtor(op_type);
                }
                FC_ASSERT((aobj.allowed_operations.size() + op.allowed_operations_to_add.size() - op.allowed_operations_to_remove.size()) > 0, "Allowed operations should be positive");

                for (const auto &acc : op.accounts_to_add)
                {
                    acc(d);
                }
                FC_ASSERT((aobj.whitelisted_accounts.size() + op.accounts_to_add.size() - op.accounts_to_remove.size()) > 0, "Accounts should be positive");

                return void_result();
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

        void_result account_role_update_evaluator::do_apply(const account_role_update_operation &op)
        {
            try
            {
                database &d = db();
                const account_role_object &aobj = op.account_role_id(d);
                d.modify(aobj, [&op](account_role_object &obj) {
                    if (op.name)
                        obj.name = *op.name;
                    if (op.metadata)
                        obj.metadata = *op.metadata;
                    obj.allowed_operations.insert(op.allowed_operations_to_add.begin(), op.allowed_operations_to_add.end());
                    obj.whitelisted_accounts.insert(op.accounts_to_add.begin(), op.accounts_to_add.end());
                    for (const auto &op_type : op.allowed_operations_to_remove)
                        obj.allowed_operations.erase(op_type);
                    for (const auto &acc : op.accounts_to_remove)
                        obj.whitelisted_accounts.erase(acc);
                });
                return void_result();
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

        void_result account_role_delete_evaluator::do_evaluate(const account_role_delete_operation &op)
        {
            try
            {
                const database &d = db();
                auto now = d.head_block_time();
                FC_ASSERT(now >= HARDFORK_NFT_TIME, "Not allowed until NFT HF");
                op.owner(d);
                const account_role_object &aobj = op.account_role_id(d);
                FC_ASSERT(aobj.owner == op.owner, "Only owner account can delete account role object");
                return void_result();
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

        void_result account_role_delete_evaluator::do_apply(const account_role_delete_operation &op)
        {
            try
            {
                database &d = db();
                const account_role_object &aobj = op.account_role_id(d);
                // TODO: Remove from Resource Objects
                d.remove(aobj);
                return void_result();
            }
            FC_CAPTURE_AND_RETHROW((op))
        }

    } // namespace chain
} // namespace graphene
