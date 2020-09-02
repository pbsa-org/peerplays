#pragma once
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>

namespace graphene
{
    namespace chain
    {
        using namespace graphene::db;

        class account_role_object : public abstract_object<account_role_object>
        {
        public:
            static const uint8_t space_id = protocol_ids;
            static const uint8_t type_id = account_role_type;

            account_id_type owner;
            std::string name;
            std::string metadata;
            flat_set<int> allowed_operations;
            flat_set<account_id_type> whitelisted_accounts;
        };

        struct by_owner;
        using account_role_multi_index_type = multi_index_container<
            account_role_object,
            indexed_by<
                ordered_unique< tag<by_id>, 
                                    member<object, object_id_type, &object::id>
                >,
                ordered_non_unique< tag<by_owner>, 
                                    member<account_role_object, account_id_type, &account_role_object::owner>
                >
            >
        >;
        using account_role_index = generic_index<account_role_object, account_role_multi_index_type>;
    } // namespace chain
} // namespace graphene

FC_REFLECT_DERIVED(graphene::chain::account_role_object, (graphene::db::object),
                   (owner)(name)(metadata)(allowed_operations)(whitelisted_accounts))